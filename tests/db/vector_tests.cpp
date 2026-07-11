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

// A read-only / immutable open cannot write, so reconcile must be skipped rather
// than crash. The index built on the earlier writable open answers knn queries.
TEST_CASE("knn on a read-only open", "[vector]") {
    barq_path path;

    {
        barq::native::db_config config;
        config.set_path(path);
        auto barq = db(config);
        barq.write([&barq] {
            auto d = VectorDoc();
            d._id = 1;
            d.embedding = {1.0f, 0.0f, 0.0f, 0.0f};
            return barq.add(std::move(d));
        });
    }

    barq::native::db_config config;
    config.set_path(path);
    config.set_schema_mode(barq::native::db_config::schema_mode::immutable);
    auto barq = db(config); // must not attempt a write during reconcile

    auto nearest = barq.objects<VectorDoc>().knn(
        &VectorDoc::embedding,
        std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
        knn_options::approximate(1));
    REQUIRE(nearest.size() == 1);
    CHECK(nearest[0]._id == 1);
}

// A knn result set is live: observing it reports changes as the nearest set
// shifts, and a frozen copy keeps the knn order and stops tracking the db.
TEST_CASE("live and frozen knn results", "[vector]") {
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
        add_vec(2, {0.9f, 0.1f, 0.0f, 0.0f});
        add_vec(3, {0.0f, 1.0f, 0.0f, 0.0f});
    });

    auto near = barq.objects<VectorDoc>().knn(
        &VectorDoc::embedding,
        std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
        knn_options::approximate(2));

    SECTION("observe fires when the nearest object is removed") {
        int calls = 0;
        auto token = near.observe([&calls](auto&&) { calls++; });
        barq.refresh(); // initial notification

        barq.write([&barq] {
            auto r = barq.objects<VectorDoc>().where([](auto&& d) { return d._id == 1; });
            auto o = r[0];
            barq.remove(o);
        });
        barq.refresh(); // change notification

        CHECK(calls == 2);
        CHECK(near.size() == 2); // still the two nearest, now ids 2 and 3
    }

    SECTION("a frozen knn result keeps its order and stops tracking the db") {
        auto frozen = near.freeze();
        REQUIRE(frozen.size() == 2);
        CHECK(frozen[0]._id == 1); // closest first
        CHECK(frozen[1]._id == 2);

        barq.write([&barq] {
            auto r = barq.objects<VectorDoc>().where([](auto&& d) { return d._id == 1; });
            auto o = r[0];
            barq.remove(o);
        });

        // The live set moved on; the frozen snapshot did not.
        CHECK(frozen.size() == 2);
        CHECK(frozen[0]._id == 1);
    }
}
