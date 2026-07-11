#include "../main.hpp"
#include "test_objects.hpp"

using namespace barq::native;

namespace barq::native {
    struct VectorDoc {
        barq::native::primary_key<int64_t> _id;
        std::string text;
        vector_indexed<4, vector_metric::cosine> embedding;
    };
    BARQ_SCHEMA(VectorDoc, _id, text, embedding)
}

// Storage behaviour of a vector_indexed<> property. It stores a list of floats
// and is assigned / read as a whole vector. The knn search itself (which needs
// the index created at open time) is covered separately once reconcile lands.
TEST_CASE("vector_indexed property", "[vector]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);

    SECTION("stores and reads a whole embedding") {
        auto doc = VectorDoc();
        doc._id = 1;
        doc.text = "hello";
        doc.embedding = std::vector<float>{0.5f, -0.5f, 0.25f, 0.75f};

        auto barq = db(config);
        auto managed_doc = barq.write([&barq, &doc] {
            return barq.add(std::move(doc));
        });

        CHECK(managed_doc.embedding.size() == 4);
        CHECK(managed_doc.embedding[0] == 0.5f);
        CHECK(managed_doc.embedding[3] == 0.75f);
        CHECK(managed_doc.embedding.value() == std::vector<float>{0.5f, -0.5f, 0.25f, 0.75f});

        // Whole-vector reassignment replaces the embedding.
        barq.write([&managed_doc] {
            managed_doc.embedding = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f};
        });
        CHECK(managed_doc.embedding.value() == std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f});
    }

    SECTION("detach reconstructs the unmanaged object") {
        auto doc = VectorDoc();
        doc._id = 2;
        doc.text = "world";
        doc.embedding = {0.1f, 0.2f, 0.3f, 0.4f};

        auto barq = db(config);
        auto managed_doc = barq.write([&barq, &doc] {
            return barq.add(std::move(doc));
        });

        VectorDoc detached = managed_doc.detach();
        CHECK(detached.text == "world");
        CHECK(detached.embedding.value == std::vector<float>{0.1f, 0.2f, 0.3f, 0.4f});
    }

    SECTION("embedding persists across reopen") {
        {
            auto doc = VectorDoc();
            doc._id = 3;
            doc.embedding = {0.6f, 0.8f, 0.0f, 0.0f};
            auto barq = db(config);
            barq.write([&barq, &doc] {
                return barq.add(std::move(doc));
            });
        }
        auto reopened = db(config);
        auto res = reopened.objects<VectorDoc>().where([](auto&& d) {
            return d._id == 3;
        });
        REQUIRE(res.size() == 1);
        CHECK(res[0].embedding.value() == std::vector<float>{0.6f, 0.8f, 0.0f, 0.0f});
    }
}

// End-to-end knn: opening the db reconciles the vector index for VectorDoc's
// embedding, and knn() then returns the nearest objects ordered by distance.
TEST_CASE("knn search", "[vector]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    auto barq = db(config);

    barq.write([&barq] {
        auto add_vec = [&](int64_t id, std::vector<float> v) {
            auto d = VectorDoc();
            d._id = id;
            d.embedding = std::move(v);
            barq.add(std::move(d));
        };
        add_vec(1, {1.0f, 0.0f, 0.0f, 0.0f});
        add_vec(2, {0.0f, 1.0f, 0.0f, 0.0f});
        add_vec(3, {0.0f, 0.0f, 1.0f, 0.0f});
        add_vec(4, {0.9f, 0.1f, 0.0f, 0.0f}); // nearly the same direction as id 1
    });

    SECTION("approximate knn returns the k nearest by cosine, closest first") {
        auto nearest = barq.objects<VectorDoc>().knn(
            &VectorDoc::embedding,
            std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
            knn_options::approximate(2));

        REQUIRE(nearest.size() == 2);
        CHECK(nearest[0]._id == 1); // exact direction match
        CHECK(nearest[1]._id == 4); // next closest by cosine
    }

    SECTION("exact knn returns the true nearest neighbour") {
        auto nearest = barq.objects<VectorDoc>().knn(
            &VectorDoc::embedding,
            std::vector<float>{0.0f, 1.0f, 0.0f, 0.0f},
            knn_options::exact(1));

        REQUIRE(nearest.size() == 1);
        CHECK(nearest[0]._id == 2);
    }

    SECTION("the index survives reopen (no rebuild needed)") {
        auto reopened = db(config);
        auto nearest = reopened.objects<VectorDoc>().knn(
            &VectorDoc::embedding,
            std::vector<float>{0.0f, 0.0f, 1.0f, 0.0f},
            knn_options::approximate(1));
        REQUIRE(nearest.size() == 1);
        CHECK(nearest[0]._id == 3);
    }
}
