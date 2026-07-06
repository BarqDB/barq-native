#include "../main.hpp"
#include "test_objects.hpp"

#include "barq/uuid.hpp"

using namespace barq::native;

TEST_CASE("uuid", "[uuid]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    SECTION("unmanaged_managed_uuid") {
        auto barq = db(std::move(config));
        std::string uuid_str = "e621e1f8-c36c-495a-93fc-0c247a3e6e5f";
        auto core_uuid = barq::UUID("e621e1f8-c36c-495a-93fc-0c247a3e6e5f");

        auto uuid1 = barq::native::uuid();
        CHECK(uuid1.to_string() == "00000000-0000-0000-0000-000000000000");
        auto uuid2 = barq::native::uuid(uuid_str);
        CHECK(uuid2.to_string() == "e621e1f8-c36c-495a-93fc-0c247a3e6e5f");
        auto uuid3 = barq::native::uuid(core_uuid.to_bytes());
        CHECK(uuid3.to_string() == "e621e1f8-c36c-495a-93fc-0c247a3e6e5f");
        CHECK(uuid3.to_bytes() == core_uuid.to_bytes());

        CHECK(uuid2 == uuid3);
        CHECK(uuid2 != uuid1);

        auto object = AllTypesObject();
        object.uuid_col = uuid2;
        object.opt_uuid_col = uuid2;
        object.list_uuid_col.push_back(uuid2);
        object.map_uuid_col = {
                {"a", uuid2}
        };

        CHECK(object.uuid_col == uuid2);
        CHECK(*object.opt_uuid_col == uuid2);
        CHECK(object.list_uuid_col[0] == uuid2);
        CHECK(uuid2 == object.map_uuid_col["a"]);

        auto managed_obj = barq.write([&] {
            return barq.add(std::move(object));
        });

        CHECK(managed_obj.uuid_col == uuid2);
        CHECK(*managed_obj.opt_uuid_col == uuid2);
        CHECK(managed_obj.list_uuid_col[0] == uuid2);
        CHECK(managed_obj.map_uuid_col["a"] == uuid2);

        CHECK(managed_obj.uuid_col != uuid1);
        CHECK(*managed_obj.opt_uuid_col != uuid1);
        CHECK(managed_obj.list_uuid_col[0] != uuid1);
        CHECK(managed_obj.map_uuid_col["a"] != uuid1);
    }
}
