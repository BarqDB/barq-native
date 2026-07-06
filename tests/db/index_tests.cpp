#include "../main.hpp"

namespace barq::native {

    struct IndexedObject {
        primary_key<int64_t> _id;
        indexed<int64_t> score;
        indexed<std::string> email;
        fulltext<std::string> bio;
        std::string name;
    };
    BARQ_SCHEMA(IndexedObject, _id, score, email, bio, name)

    TEST_CASE("column index type") {
        SECTION("declared schema carries index flags", "[index]") {
            auto object_schema = managed<IndexedObject>::schema.to_core_schema();

            CHECK(object_schema.property_for_name("score").is_indexed());
            CHECK_FALSE(object_schema.property_for_name("score").is_fulltext_indexed());

            CHECK(object_schema.property_for_name("email").is_indexed());
            CHECK_FALSE(object_schema.property_for_name("email").is_fulltext_indexed());

            CHECK(object_schema.property_for_name("bio").is_fulltext_indexed());
            CHECK_FALSE(object_schema.property_for_name("bio").is_indexed());

            CHECK_FALSE(object_schema.property_for_name("name").is_indexed());
            CHECK_FALSE(object_schema.property_for_name("name").is_fulltext_indexed());

            // A primary key is indexed by core, but not through our is_indexed flag.
            CHECK_FALSE(object_schema.property_for_name("_id").is_indexed());
        }

        SECTION("opening a db builds the indexes on disk", "[index]") {
            barq_path path;
            db_config config;
            config.set_path(path);
            auto barq = db(std::move(config));

            auto live_schema = barq.m_barq.schema();
            auto object_schema = live_schema.find("IndexedObject");

            CHECK(object_schema.property_for_name("score").is_indexed());
            CHECK(object_schema.property_for_name("email").is_indexed());
            CHECK(object_schema.property_for_name("bio").is_fulltext_indexed());
            CHECK_FALSE(object_schema.property_for_name("name").is_indexed());
            CHECK_FALSE(object_schema.property_for_name("name").is_fulltext_indexed());
        }

        SECTION("indexed fields read, write and query normally", "[index]") {
            barq_path path;
            db_config config;
            config.set_path(path);
            auto barq = db(std::move(config));

            auto make = [&](int64_t id, int64_t score, const char* email, const char* bio, const char* name) {
                IndexedObject object;
                object._id = id;
                object.score = score;
                object.email = email;
                object.bio = bio;
                object.name = name;
                return barq.write([&] { return barq.add(std::move(object)); });
            };

            auto ada = make(1, 42, "ada@example.test", "loves offline first databases", "Ada");
            make(2, 7, "grace@example.test", "compiler pioneer", "Grace");

            // Values round-trip through the index wrappers.
            std::string email = ada.email;
            int64_t score = ada.score;
            CHECK(email == "ada@example.test");
            CHECK(score == 42);

            CHECK(barq.objects<IndexedObject>().size() == 2);

            // Equality query on a general-indexed string column.
            CHECK(barq.objects<IndexedObject>().where([](auto& o) {
                return o.email == "ada@example.test";
            }).size() == 1);

            // Range query on a general-indexed integer column.
            CHECK(barq.objects<IndexedObject>().where([](auto& o) {
                return o.score > 10;
            }).size() == 1);
        }
    }
}
