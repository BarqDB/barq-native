#include "../main.hpp"
#include "test_objects.hpp"

using namespace barq::native;

TEST_CASE("object_id", "[object_id]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    SECTION("unmanaged_managed_object_id", "[object_id]") {
        auto barq = db(std::move(config));
        auto object_id1 = barq::native::object_id::generate();
        auto object_id2 = barq::native::object_id::generate();
        auto object_id3 = barq::native::object_id::generate();
        auto object_id4 = barq::native::object_id::generate();
        auto object = AllTypesObject();
        object.object_id_col = object_id1;
        object.opt_object_id_col = object_id2;
        object.list_object_id_col.push_back(object_id3);
        object.map_object_id_col = {
                {"a", object_id4}
        };

        CHECK(object.object_id_col == object_id1);
        CHECK(*object.opt_object_id_col == object_id2);
        CHECK(object.list_object_id_col[0] == object_id3);
        CHECK(object_id4 == object.map_object_id_col["a"]);

        auto managed_obj = barq.write([&] {
            return barq.add(std::move(object));
        });

        CHECK(managed_obj.object_id_col == object_id1);
        CHECK(*managed_obj.opt_object_id_col == object_id2);
        CHECK(managed_obj.list_object_id_col[0] == object_id3);
        CHECK(managed_obj.map_object_id_col["a"] == object_id4);

        const std::string val = "000123450000ffbeef91906c";
        auto object_id5 = barq::native::object_id(val);
        CHECK(object_id5.to_string() == val);
    }
}