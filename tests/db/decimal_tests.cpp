#include "../main.hpp"
#include "test_objects.hpp"

using namespace barq::native;

TEST_CASE("decimal128_tests", "[decimal128]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    SECTION("decimal_basics", "[decimal128]") {
        auto barq = db(std::move(config));
        auto decimal1 = barq::native::decimal128(123.456);
        auto decimal2 = barq::native::decimal128("123.456");
        auto decimal3 = barq::native::decimal128();
        auto decimal4 = barq::native::decimal128("NaN");
        auto object = AllTypesObject();
        object.decimal_col = decimal1;
        object.opt_decimal_col = decimal2;
        object.list_decimal_col.push_back(decimal3);
        object.map_decimal_col = {
                {"a", decimal4}
        };

        CHECK(object.decimal_col == decimal1);
        CHECK(*object.opt_decimal_col == decimal2);
        CHECK(object.list_decimal_col[0] == decimal3);
        CHECK(decimal4 == object.map_decimal_col["a"]);

        auto managed_obj = barq.write([&] {
            return barq.add(std::move(object));
        });

        CHECK(managed_obj.decimal_col == decimal1);
        CHECK(*managed_obj.opt_decimal_col == decimal2);
        CHECK(managed_obj.list_decimal_col[0] == decimal3);
        CHECK(managed_obj.map_decimal_col["a"] == decimal4);
    }

    SECTION("is_NaN", "[decimal128]") {
        auto barq = db(std::move(config));
        auto decimal = barq::native::decimal128("NaN");
        auto object = AllTypesObject();
        object.decimal_col = decimal;
        auto managed_obj = barq.write([&] {
            return barq.add(std::move(object));
        });
        CHECK((*managed_obj.decimal_col).is_NaN());
    }

    SECTION("is_NaN_optional", "[decimal128]") {
        auto barq = db(std::move(config));
        auto decimal = barq::native::decimal128("NaN");
        auto object = AllTypesObject();
        object.opt_decimal_col = decimal;
        auto managed_obj = barq.write([&] {
            return barq.add(std::move(object));
        });
        CHECK((*managed_obj.opt_decimal_col)->is_NaN());
    }

    SECTION("arithmetic", "[decimal128]") {
        auto barq = db(std::move(config));
        auto decimal = barq::native::decimal128(10);
        auto object = AllTypesObject();
        object.decimal_col = decimal;
        auto managed_obj = barq.write([&] {
            return barq.add(std::move(object));
        });

        // Addition
        barq.write([&] {
            managed_obj.decimal_col = managed_obj.decimal_col + barq::native::decimal128(20);
            CHECK((*managed_obj.decimal_col).to_string() == "30");
            managed_obj.decimal_col = managed_obj.decimal_col + barq::native::decimal128(-20);
            CHECK((*managed_obj.decimal_col).to_string() == "10");
            managed_obj.decimal_col += barq::native::decimal128(20);
            CHECK((*managed_obj.decimal_col).to_string() == "30");
        });
        // Subtraction
        barq.write([&] {
            managed_obj.decimal_col = managed_obj.decimal_col - barq::native::decimal128(15);
            CHECK((*managed_obj.decimal_col).to_string() == "15");
            managed_obj.decimal_col = managed_obj.decimal_col - barq::native::decimal128(-15);
            CHECK((*managed_obj.decimal_col).to_string() == "30");
            managed_obj.decimal_col -= barq::native::decimal128(20);
            CHECK((*managed_obj.decimal_col).to_string() == "10");
        });
        // Division
        barq.write([&] {
            managed_obj.decimal_col = managed_obj.decimal_col / barq::native::decimal128(4);
            CHECK((*managed_obj.decimal_col).to_string() == "2.5");
            managed_obj.decimal_col = managed_obj.decimal_col / barq::native::decimal128(2.5);
            CHECK((*managed_obj.decimal_col).to_string() == "1");
            managed_obj.decimal_col = barq::native::decimal128(6);
            managed_obj.decimal_col /= barq::native::decimal128(2);
            CHECK((*managed_obj.decimal_col).to_string() == "3");
        });
        // Multiplication
        barq.write([&] {
            managed_obj.decimal_col = managed_obj.decimal_col * barq::native::decimal128(4);
            CHECK((*managed_obj.decimal_col).to_string() == "12");
            managed_obj.decimal_col = managed_obj.decimal_col * barq::native::decimal128(-2);
            CHECK((*managed_obj.decimal_col).to_string() == "-24");
            managed_obj.decimal_col *= barq::native::decimal128(2);
            CHECK((*managed_obj.decimal_col).to_string() == "-48");
        });
    }

    SECTION("arithmetic_optional", "[decimal128]") {
        auto barq = db(std::move(config));
        auto decimal = barq::native::decimal128(10);
        auto object = AllTypesObject();
        object.opt_decimal_col = decimal;
        auto managed_obj = barq.write([&] {
            return barq.add(std::move(object));
        });

        // Addition
        barq.write([&] {
            managed_obj.opt_decimal_col = managed_obj.opt_decimal_col + barq::native::decimal128(20);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "30");
            managed_obj.opt_decimal_col = managed_obj.opt_decimal_col + barq::native::decimal128(-20);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "10");
            managed_obj.opt_decimal_col += barq::native::decimal128(20);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "30");
        });
        // Subtraction
        barq.write([&] {
            managed_obj.opt_decimal_col = managed_obj.opt_decimal_col - barq::native::decimal128(15);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "15");
            managed_obj.opt_decimal_col = managed_obj.opt_decimal_col - barq::native::decimal128(-15);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "30");
            managed_obj.opt_decimal_col -= barq::native::decimal128(20);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "10");
        });
        // Division
        barq.write([&] {
            managed_obj.opt_decimal_col = managed_obj.opt_decimal_col / barq::native::decimal128(4);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "2.5");
            managed_obj.opt_decimal_col = managed_obj.opt_decimal_col / barq::native::decimal128(2.5);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "1");
            managed_obj.opt_decimal_col = barq::native::decimal128(6);
            managed_obj.opt_decimal_col /= barq::native::decimal128(2);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "3");
        });
        // Multiplication
        barq.write([&] {
            managed_obj.opt_decimal_col = managed_obj.opt_decimal_col * barq::native::decimal128(4);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "12");
            managed_obj.opt_decimal_col = managed_obj.opt_decimal_col * barq::native::decimal128(-2);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "-24");
            managed_obj.opt_decimal_col *= barq::native::decimal128(2);
            CHECK((*managed_obj.opt_decimal_col)->to_string() == "-48");
        });
    }

    SECTION("predicates", "[decimal128]") {
        auto barq = db(std::move(config));
        auto decimal = barq::native::decimal128("123.456");
        auto object = AllTypesObject();
        object.decimal_col = decimal;

        CHECK(object.decimal_col > barq::native::decimal128("1"));
        CHECK(object.decimal_col >= barq::native::decimal128("1"));
        CHECK(object.decimal_col == barq::native::decimal128("123.456"));
        CHECK(object.decimal_col != barq::native::decimal128("1"));
        CHECK(object.decimal_col < barq::native::decimal128("200.0"));
        CHECK(object.decimal_col <= barq::native::decimal128("200.0"));

        auto managed_obj = barq.write([&] {
            return barq.add(std::move(object));
        });

        CHECK(managed_obj.decimal_col > barq::native::decimal128("1"));
        CHECK(managed_obj.decimal_col >= barq::native::decimal128("1"));
        CHECK(managed_obj.decimal_col == barq::native::decimal128("123.456"));
        CHECK(managed_obj.decimal_col != barq::native::decimal128("1"));
        CHECK(managed_obj.decimal_col < barq::native::decimal128("200.0"));
        CHECK(managed_obj.decimal_col <= barq::native::decimal128("200.0"));

        CHECK(barq.objects<AllTypesObject>().where([](auto& obj) { return obj.decimal_col > barq::native::decimal128("1"); }).size() == 1);
        CHECK(barq.objects<AllTypesObject>().where([](auto& obj) { return obj.decimal_col >= barq::native::decimal128("1"); }).size() == 1);
        CHECK(barq.objects<AllTypesObject>().where([](auto& obj) { return obj.decimal_col == barq::native::decimal128("123.456"); }).size() == 1);
        CHECK(barq.objects<AllTypesObject>().where([](auto& obj) { return obj.decimal_col != barq::native::decimal128("123.456"); }).size() == 0);
        CHECK(barq.objects<AllTypesObject>().where([](auto& obj) { return obj.decimal_col < barq::native::decimal128("200.0"); }).size() == 1);
        CHECK(barq.objects<AllTypesObject>().where([](auto& obj) { return obj.decimal_col <= barq::native::decimal128("200.0"); }).size() == 1);
    }
}