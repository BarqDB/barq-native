#include "../main.hpp"
#include "test_objects.hpp"

using namespace barq::native;

TEST_CASE("mixed", "[mixed]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    SECTION("unmanaged_managed_mixed_get_set", "[mixed]") {
        auto obj = AllTypesObject();
        obj.mixed_col = (int64_t)42;
        CHECK(obj.mixed_col == barq::native::mixed((int64_t)42));
        auto barq = db(std::move(config));
        auto managed_obj = barq.write([&barq, &obj] {
            return barq.add(std::move(obj));
        });
        CHECK(managed_obj.mixed_col == static_cast<int64_t>(42));
        barq.write([&managed_obj] {
            managed_obj.mixed_col = std::string("hello world");
        });
        CHECK(managed_obj.mixed_col == std::string("hello world"));
        barq.write([&managed_obj] {
            managed_obj.mixed_col = true;
        });
        CHECK(managed_obj.mixed_col == true);
        auto time_point = std::chrono::time_point<std::chrono::system_clock>();
        barq.write([&managed_obj, &time_point] {
            managed_obj.mixed_col = time_point;
        });
        CHECK(managed_obj.mixed_col == time_point);
        auto bin = std::vector<uint8_t>{0,1,2};
        barq.write([&managed_obj, &bin] {
            managed_obj.mixed_col = bin;
        });
        CHECK(managed_obj.mixed_col == bin);
        barq.write([&managed_obj] {
            managed_obj.mixed_col = 42.42;
        });
        CHECK(managed_obj.mixed_col == 42.42);
        auto u = uuid();
        barq.write([&managed_obj, u] {
            managed_obj.mixed_col = u;
        });
        CHECK(managed_obj.mixed_col == u);
    }

    SECTION("holds_alternative") {
        auto obj = AllTypesObject();
        obj.mixed_col = (int64_t)42;
        CHECK(obj.mixed_col == barq::native::mixed((int64_t)42));
        auto barq = db(std::move(config));
        auto managed_obj = barq.write([&barq, &obj] {
            return barq.add(std::move(obj));
        });
        bool result = barq::native::holds_alternative<int64_t>(managed_obj.mixed_col);
        CHECK(result);
        result = barq::native::holds_alternative<bool>(managed_obj.mixed_col);
        CHECK_FALSE(result);

        barq.write([&barq, &managed_obj] {
            managed_obj.mixed_col = std::string("foo");
        });
        result = barq::native::holds_alternative<std::string>(managed_obj.mixed_col);
        CHECK(result);

        barq.write([&barq, &managed_obj] {
            managed_obj.mixed_col = std::vector<uint8_t>({1,1,1,1});
        });
        result = barq::native::holds_alternative<std::vector<uint8_t>>(managed_obj.mixed_col);
        CHECK(result);

        barq.write([&barq, &managed_obj] {
            managed_obj.mixed_col = barq::native::mixed((int64_t)1234);
        });
        result = barq::native::holds_alternative<int64_t>(managed_obj.mixed_col);
        CHECK(result);

        barq.write([&barq, &managed_obj] {
            managed_obj.mixed_col = std::chrono::time_point<std::chrono::system_clock>();
        });
        result = barq::native::holds_alternative<std::chrono::time_point<std::chrono::system_clock>>(managed_obj.mixed_col);
        CHECK(result);

        barq.write([&barq, &managed_obj] {
            managed_obj.mixed_col = 123.456;
        });
        result = barq::native::holds_alternative<double>(managed_obj.mixed_col);
        CHECK(result);

        barq.write([&barq, &managed_obj] {
            managed_obj.mixed_col = barq::native::decimal128("123.456");
        });
        result = barq::native::holds_alternative<barq::native::decimal128>(managed_obj.mixed_col);
        CHECK(result);

        barq.write([&barq, &managed_obj] {
            managed_obj.mixed_col = barq::native::object_id();
        });
        result = barq::native::holds_alternative<barq::native::object_id>(managed_obj.mixed_col);
        CHECK(result);

        barq.write([&barq, &managed_obj] {
            managed_obj.mixed_col = barq::native::uuid();
        });
        result = barq::native::holds_alternative<barq::native::uuid>(managed_obj.mixed_col);
        CHECK(result);

        barq.write([&barq, &managed_obj] {
            managed_obj.mixed_col = std::monostate();
        });
        result = barq::native::holds_alternative<std::monostate>(managed_obj.mixed_col);
        CHECK(result);
    }

    SECTION("links") {
        auto obj = AllTypesObject();
        obj.mixed_col = (int64_t)42;
        CHECK(obj.mixed_col == barq::native::mixed((int64_t)42));
        auto barq = db(std::move(config));
        auto managed_obj = barq.write([&barq, &obj] {
            return barq.add(std::move(obj));
        });
        CHECK_FALSE(managed_obj.mixed_col.has_link());

        auto link = AllTypesObjectLink();
        link._id = 0;
        link.str_col = "foo";

        barq.write([&] {
            managed_obj.mixed_col.set_link(std::move(link));
        });
        CHECK(managed_obj.mixed_col.has_link());

        auto mixed_link = managed_obj.mixed_col.get_stored_link<AllTypesObjectLink*>();
        CHECK(mixed_link->str_col == "foo");

        bool result = barq::native::holds_alternative<AllTypesObjectLink*>(managed_obj.mixed_col);
        CHECK(result);

        result = barq::native::holds_alternative<StringObject*>(managed_obj.mixed_col);
        CHECK_FALSE(result);

        auto link2 = AllTypesObjectLink();
        link2._id = 0;
        link2.str_col = "bar";

        auto managed_link = barq.write([&] {
            return barq.add(std::move(link2));
        });

        barq.write([&] {
            managed_obj.mixed_col.set_link(managed_link);
        });
        mixed_link = managed_obj.mixed_col.get_stored_link<AllTypesObjectLink*>();
        CHECK(mixed_link->str_col == "bar");

        CHECK_THROWS(managed_obj.mixed_col.get_stored_link<StringObject*>());
    }
}