#include "../main.hpp"
#include "test_objects.hpp"

namespace barq::native {

    TEST_CASE("string", "[str]") {
        barq_path path;
        barq::native::db_config config;
        config.set_path(path);

        SECTION("managed_str_get_set", "[str]") {
            auto obj = AllTypesObject();
            auto barq = db(std::move(config));
            auto managed_obj = barq.write([&barq, &obj] {
                return barq.add(std::move(obj));
            });
            CHECK_THROWS(managed_obj.str_col = "foo");
            CHECK(managed_obj.str_col == "");
            barq.write([&managed_obj] { managed_obj.str_col = "bar"; });
            CHECK(managed_obj.str_col == "bar");
        }

        SECTION("managed_str_contains", "[str]") {
            auto obj = AllTypesObject();
            auto barq = db(std::move(config));
            auto managed_obj = barq.write([&barq, &obj] {
                obj.str_col = "foo";
                return barq.add(std::move(obj));
            });
            CHECK(managed_obj.str_col.contains("oo"));
        }
    }
}