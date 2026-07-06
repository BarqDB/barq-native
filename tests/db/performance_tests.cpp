#include "../main.hpp"
#include "test_objects.hpp"

using namespace barq::native;

TEST_CASE("basic_performance", "[performance]") {
    BENCHMARK_ADVANCED("write 1000")(Catch::Benchmark::Chronometer meter) {
        barq_path path;
        barq::native::db_config config;
        config.set_path(path);
        auto barq = db(std::move(config));

        return meter.measure([&]() {
            barq.write([&] {
                for (int64_t i = 0; i < 1000; i++) {
                    AllTypesObject o;
                    o._id = i;
                    barq.add(std::move(o));
                }
            });
        });
        CHECK(barq.objects<AllTypesObject>().size() == 1000);
    };

    BENCHMARK_ADVANCED("read 1000")(Catch::Benchmark::Chronometer meter) {
        barq_path path;
        barq::native::db_config config;
        config.set_path(path);
        auto barq = db(std::move(config));

        barq.write([&] {
            for (int64_t i = 0; i < 1000; i++) {
                AllTypesObject o;
                o._id = i;
                barq.add(std::move(o));
            }
        });

        return meter.measure([&]() {
            auto results = barq.objects<AllTypesObject>();
            CHECK(results.size() == 1000);
            for (int64_t i = 0; i < 1000; i++) {
                CHECK(results[i]._id == i);
            }
        });
    };

    BENCHMARK_ADVANCED("write 10000")(Catch::Benchmark::Chronometer meter) {
        barq_path path;
        barq::native::db_config config;
        config.set_path(path);
        auto barq = db(std::move(config));

        return meter.measure([&]() {
            barq.write([&] {
                for (int64_t i = 0; i < 10000; i++) {
                    AllTypesObject o;
                    o._id = i;
                    barq.add(std::move(o));
                }
            });
        });
        CHECK(barq.objects<AllTypesObject>().size() == 10000);

    };

    BENCHMARK_ADVANCED("read 10000")(Catch::Benchmark::Chronometer meter) {
        barq_path path;
        barq::native::db_config config;
        config.set_path(path);
        auto barq = db(std::move(config));

        barq.write([&] {
            for (int64_t i = 0; i < 10000; i++) {
                AllTypesObject o;
                o._id = i;
                barq.add(std::move(o));
            }
        });

        return meter.measure([&]() {
            auto results = barq.objects<AllTypesObject>();
            CHECK(results.size() == 10000);
            for (int64_t i = 0; i < 10000; i++) {
                CHECK(results[i]._id == i);
            }
        });
    };
}
