#include "../main.hpp"
#include "test_objects.hpp"

using namespace barq::native;

namespace barq::native {
    struct FloatObject {
        barq::native::primary_key<int64_t> _id;
        float score = 0.0f;
        std::optional<float> opt_score;
        std::vector<float> embedding;
    };
    BARQ_SCHEMA(FloatObject, _id, score, opt_score, embedding)
}

// Values below are all exactly representable in binary32, so equality checks
// are safe and prove the column is really `float` (not silently promoted to
// `double`, which would round-trip differently for non-exact values).
TEST_CASE("float", "[float]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);

    SECTION("scalar float round-trips") {
        auto obj = FloatObject();
        obj._id = 1;
        obj.score = 1.5f;
        obj.opt_score = -0.25f;

        auto barq = db(config);
        auto managed_obj = barq.write([&barq, &obj] {
            return barq.add(std::move(obj));
        });

        CHECK(static_cast<float>(managed_obj.score) == 1.5f);
        CHECK(static_cast<std::optional<float>>(managed_obj.opt_score) == std::optional<float>(-0.25f));

        barq.write([&managed_obj] {
            managed_obj.score = 3.25f;
            managed_obj.score += 0.5f;
        });
        CHECK(static_cast<float>(managed_obj.score) == 3.75f);
    }

    SECTION("std::vector<float> round-trips") {
        auto obj = FloatObject();
        obj._id = 2;
        obj.embedding = {1.0f, 2.5f, -3.25f, 0.0f};

        auto barq = db(config);
        auto managed_obj = barq.write([&barq, &obj] {
            return barq.add(std::move(obj));
        });

        std::vector<float> got = managed_obj.embedding.detach();
        CHECK(got == std::vector<float>{1.0f, 2.5f, -3.25f, 0.0f});
        CHECK(managed_obj.embedding.size() == 4);
        CHECK(managed_obj.embedding[0] == 1.0f);
        CHECK(managed_obj.embedding[2] == -3.25f);

        // Incremental edits on the managed list go through the float bridge.
        barq.write([&managed_obj] {
            managed_obj.embedding.push_back(9.5f);
            managed_obj.embedding.set(0, 11.0f);
        });
        CHECK(managed_obj.embedding.size() == 5);
        CHECK(managed_obj.embedding[0] == 11.0f);
        CHECK(managed_obj.embedding[4] == 9.5f);
        CHECK(managed_obj.embedding.find(9.5f) == 4);
    }

    SECTION("query on a float column") {
        auto obj = FloatObject();
        obj._id = 3;
        obj.score = 42.5f;

        auto barq = db(config);
        barq.write([&barq, &obj] {
            return barq.add(std::move(obj));
        });

        CHECK(barq.objects<FloatObject>().where([](auto&& o) {
            return o.score == 42.5f;
        }).size() == 1);
        CHECK(barq.objects<FloatObject>().where([](auto&& o) {
            return o.score > 100.0f;
        }).size() == 0);
        CHECK(barq.objects<FloatObject>().where([](auto&& o) {
            return o.score < 100.0f;
        }).size() == 1);
    }

    SECTION("float data persists across reopen") {
        {
            auto obj = FloatObject();
            obj._id = 4;
            obj.score = 7.75f;
            obj.embedding = {0.5f, 0.5f, 0.25f};
            auto barq = db(config);
            barq.write([&barq, &obj] {
                return barq.add(std::move(obj));
            });
        }
        auto reopened = db(config);
        auto res = reopened.objects<FloatObject>().where([](auto&& o) {
            return o._id == 4;
        });
        REQUIRE(res.size() == 1);
        auto row = res[0];
        CHECK(static_cast<float>(row.score) == 7.75f);
        CHECK(row.embedding.detach() == std::vector<float>{0.5f, 0.5f, 0.25f});
    }
}
