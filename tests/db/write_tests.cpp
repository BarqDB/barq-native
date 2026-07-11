#include "../main.hpp"
#include "test_objects.hpp"

#include <stdexcept>

using namespace barq::native;

TEST_CASE("write rollback", "[write]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    auto barq = db(config);

    SECTION("a throwing write rolls back and leaves the barq usable") {
        // A write whose body throws after adding an object must not persist
        // that object, and must not leave the transaction open.
        CHECK_THROWS_AS(barq.write([&barq] {
            auto o = StringObject();
            o._id = 100;
            o.str_col = "doomed";
            barq.add(std::move(o));
            throw std::runtime_error("boom");
        }), std::runtime_error);

        // The doomed object was rolled back.
        CHECK(barq.objects<StringObject>().where([](auto&& o) {
            return o._id == 100;
        }).size() == 0);

        // The barq is not stuck in an open transaction: a fresh write commits.
        barq.write([&barq] {
            auto o = StringObject();
            o._id = 101;
            o.str_col = "kept";
            return barq.add(std::move(o));
        });
        auto res = barq.objects<StringObject>().where([](auto&& o) {
            return o._id == 101;
        });
        REQUIRE(res.size() == 1);
        CHECK(res[0].str_col == "kept");
    }
}
