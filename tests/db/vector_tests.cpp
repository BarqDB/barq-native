#include "../main.hpp"
#include "test_objects.hpp"

#include <barq_native/client_reset.hpp>

using namespace barq::native;

namespace barq::native {
    struct VectorDoc {
        barq::native::primary_key<int64_t> _id;
        std::string text;
        vector_indexed<4, vector_metric::cosine, vector_encoding::sq8, 8, 32, 16, 1> embedding;
    };
    BARQ_SCHEMA(VectorDoc, _id, text, embedding)

    struct PlainVectorDoc {
        barq::native::primary_key<int64_t> _id;
        std::vector<float> embedding; // deliberately not vector_indexed<>
    };
    BARQ_SCHEMA(PlainVectorDoc, _id, embedding)
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

// Editing an object's embedding re-ranks it in later knn queries.
TEST_CASE("editing an embedding updates knn", "[vector]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    auto barq = db(config);

    auto m2 = barq.write([&barq] {
        auto add = [&](int64_t id, std::vector<float> v) {
            auto d = VectorDoc();
            d._id = id;
            d.embedding = std::move(v);
            return barq.add(std::move(d));
        };
        add(1, {0.9f, 0.1f, 0.0f, 0.0f}); // close to the query
        return add(2, {0.0f, 1.0f, 0.0f, 0.0f}); // far
    });

    auto query = std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f};
    CHECK(barq.objects<VectorDoc>()
              .knn(&VectorDoc::embedding, query, knn_options::approximate(1))[0]
              ._id == 1);

    barq.write([&m2] {
        m2.embedding = std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f}; // now the exact match
    });

    CHECK(barq.objects<VectorDoc>()
              .knn(&VectorDoc::embedding, query, knn_options::approximate(1))[0]
              ._id == 2);
}

// knn composes with a query filter: it ranks only the objects that pass.
TEST_CASE("knn within a filtered subset", "[vector]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    auto barq = db(config);

    barq.write([&barq] {
        auto add = [&](int64_t id, std::vector<float> v) {
            auto d = VectorDoc();
            d._id = id;
            d.embedding = std::move(v);
            barq.add(std::move(d));
        };
        add(1, {1.0f, 0.0f, 0.0f, 0.0f}); // exact match, but filtered out below
        add(2, {0.9f, 0.1f, 0.0f, 0.0f});
        add(3, {0.0f, 1.0f, 0.0f, 0.0f});
    });

    auto near = barq.objects<VectorDoc>()
                    .where([](auto&& d) { return d._id > 1; })
                    .knn(&VectorDoc::embedding, std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
                         knn_options::approximate(2));

    REQUIRE(near.size() == 2);
    CHECK(near[0]._id == 2); // nearest among the filtered set (id 1 excluded)
    CHECK(near[1]._id == 3);
}

// The reverse chain: a where() applied to a knn result must narrow the knn
// subset (keeping its distance order), not silently rebuild from the raw table.
TEST_CASE("where() after knn() keeps the knn subset and order", "[vector]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    auto barq = db(config);

    barq.write([&barq] {
        auto add = [&](int64_t id, std::string text, std::vector<float> v) {
            auto d = VectorDoc();
            d._id = id;
            d.text = std::move(text);
            d.embedding = std::move(v);
            barq.add(std::move(d));
        };
        add(1, "skip", {1.0f, 0.0f, 0.0f, 0.0f}); // exact match, filtered out below
        add(2, "keep", {0.9f, 0.1f, 0.0f, 0.0f});
        add(3, "keep", {0.5f, 0.5f, 0.0f, 0.0f});
        add(4, "keep", {0.0f, 0.0f, 1.0f, 0.0f});
    });

    auto near_keeps = barq.objects<VectorDoc>()
                          .knn(&VectorDoc::embedding, std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
                               knn_options::exact(2))
                          .where([](auto&& d) { return d.text == "keep"; });

    // The two nearest keeps, closest first — not every keep in table order.
    REQUIRE(near_keeps.size() == 2);
    CHECK(near_keeps[0]._id == 2);
    CHECK(near_keeps[1]._id == 3);

    // A filter matching nothing empties the result instead of resurrecting rows.
    CHECK(barq.objects<VectorDoc>()
              .knn(&VectorDoc::embedding, std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
                   knn_options::exact(2))
              .where([](auto&& d) { return d.text == "nope"; })
              .size() == 0);
}

// ef_search is a query-time knob: when the persisted value drifts from the
// declared one (an older build, another SDK, a tuning experiment), the open-time
// reconcile adopts the declared value in place instead of failing the open.
TEST_CASE("a drifted ef_search is adopted in place at open", "[vector]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    {
        auto barq = db(config); // builds the index with the declared ef_search (16)
        barq.write([&barq] {
            auto d = VectorDoc();
            d._id = 1;
            d.embedding = {1.0f, 0.0f, 0.0f, 0.0f};
            barq.add(std::move(d));
        });

        // Simulate the drift through the same core call the reconcile uses.
        barq.write([&barq] {
            auto table = barq.m_barq.table_for_object_type("VectorDoc");
            auto col = table.get_column_key("embedding");
            auto drifted = table.get_vector_index_config(col);
            drifted.ef_search = 99;
            table.add_vector_index(col, drifted);
        });
        auto table = barq.m_barq.table_for_object_type("VectorDoc");
        CHECK(table.get_vector_index_config(table.get_column_key("embedding")).ef_search == 99);
    }

    // Reopen with the declared schema: no throw, and the beam is back to 16.
    auto reopened = db(config);
    auto table = reopened.m_barq.table_for_object_type("VectorDoc");
    CHECK(table.get_vector_index_config(table.get_column_key("embedding")).ef_search == 16);
    CHECK(reopened.objects<VectorDoc>()
              .knn(&VectorDoc::embedding, std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
                   knn_options::approximate(1))
              .size() == 1);
}

// The reconciler registry is program-global; opening a file that contains only
// a subset of the declared types (open<Ts...>) must skip the vector types that
// are absent instead of throwing from the constructor.
TEST_CASE("subset-schema open skips absent vector types", "[vector]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);

    auto barq = open<Dog>(config); // VectorDoc is registered but not in this file
    barq.write([&barq] {
        auto d = Dog();
        d._id = 1;
        d.name = "rex";
        barq.add(std::move(d));
    });
    CHECK(barq.objects<Dog>().size() == 1);
}

// The declared dimension count is enforced on queries: a query vector whose
// length does not match the index is rejected rather than answered wrongly.
TEST_CASE("a wrong-dimension query is rejected", "[vector]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    auto barq = db(config);

    barq.write([&barq] {
        auto d = VectorDoc();
        d._id = 1;
        d.embedding = {1.0f, 0.0f, 0.0f, 0.0f};
        return barq.add(std::move(d));
    });

    // The index is 4-dimensional (vector_indexed<4>); a 3-dim query is rejected.
    bool rejected = false;
    try {
        (void)barq.objects<VectorDoc>()
            .knn(&VectorDoc::embedding, std::vector<float>{1.0f, 0.0f, 0.0f},
                 knn_options::approximate(1))
            .size();
    }
    catch (...) {
        rejected = true;
    }
    CHECK(rejected);

    // The matching 4-dim query still works.
    CHECK(barq.objects<VectorDoc>()
              .knn(&VectorDoc::embedding, std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
                   knn_options::approximate(1))
              .size() == 1);
}

// knn needs a persisted vector index on the property. A plain float list has
// none, and the search must be refused eagerly on the calling thread — not
// deferred to the engine, whose error would surface on whatever thread
// evaluates the results (and, on older cores, was no error at all but a
// silent unranked answer).
TEST_CASE("knn on a property without a vector index is refused", "[vector]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    auto barq = db(config);

    barq.write([&barq] {
        auto d = PlainVectorDoc();
        d._id = 1;
        d.embedding = {1.0f, 0.0f, 0.0f, 0.0f};
        barq.add(std::move(d));
    });

    CHECK_THROWS_WITH(barq.objects<PlainVectorDoc>().knn(
                          &PlainVectorDoc::embedding,
                          std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
                          knn_options::approximate(1)),
                      "Property 'embedding' has no vector index");
}

// The vector index is local, so a client reset (which discards local state)
// rebuilds it via the after-reset handler. Compile-only: instantiating
// set_client_reset_handler<db> is what guards the reconcile-in-wrapper code;
// actually installing/triggering a reset needs a sync config and server (the
// sync suite), and installing it on a plain config would fault.
TEST_CASE("client reset handler for the local vector index compiles", "[vector]") {
    auto wire = [] {
        barq::native::db_config config;
        config.set_client_reset_handler(
            barq::native::client_reset::discard_unsynced_changes([](db) {}, [](db, db) {}));
    };
    (void)wire; // instantiated at compile time, never invoked
    SUCCEED("client reset handler wiring compiles");
}
