#include "../main.hpp"
#include "barq/object-store/shared_barq.hpp"
#include "test_objects.hpp"

namespace barq::native {

    TEST_CASE("cached barq") {
        SECTION("cached barq transactions") {
            barq_path path;
            barq::native::db_config config1;
            config1.set_path(path);
            auto o = managed<AllTypesObject>();
            {
                auto barq = db(std::move(config1));
                o = barq.write([&] {
                    return barq.add(AllTypesObject());
                });
            }

            barq::native::db_config config2;
            config2.set_path(path);
            {
                auto barq = db(std::move(config2));
                // If the cached barq is not returned this write would fail with a wrong transaction state.
                barq.write([&] {
                    o.str_col = "foo";
                });
            }
            CHECK(o.str_col == "foo");
        }
    }

    TEST_CASE("tsr_object") {
        barq_path path;
        barq::native::db_config config;
        config.set_path(path);
        auto barq = db(std::move(config));

        AllTypesObject obj;
        obj.str_col = "John";
        obj._id = 1;
        AllTypesObjectLink obj_link;
        obj_link.str_col = "Fido";
        obj.opt_obj_col = &obj_link;

        auto managed_obj = barq.write([&barq, &obj] {
            return barq.add(std::move(obj));
        });

        auto tsr = thread_safe_reference<AllTypesObject>(managed_obj);
        std::promise<void> p;
        auto t = std::thread([&tsr, &p, &path]() {
            barq::native::db_config config2;
            config2.set_path(path);
            auto barq = db(std::move(config2));
            auto o = barq.resolve(std::move(tsr));
            CHECK(o._id == 1);
            barq.write([&] { barq.remove(o); });
            p.set_value();
        });
        t.join();
        p.get_future().get();
    }

    TEST_CASE("encrypted barq") {
        std::array<char, 64> example_key = {0,0,0,0,0,0,0,0, 1,1,0,0,0,0,0,0, 2,2,0,0,0,0,0,0, 3,3,0,0,0,0,0,0, 4,4,0,0,0,0,0,0, 5,5,0,0,0,0,0,0, 6,6,0,0,0,0,0,0, 7,7,0,0,0,0,0,0};
        barq_path path;

        auto config = barq::native::db_config();
        config.set_encryption_key(example_key);
        config.set_path(path);
        auto barq = db(config);

        // Missing encryption key
        auto config2 = barq::native::db_config();
        config2.set_path(path);
        REQUIRE_THROWS(db(config2));
    }

    TEST_CASE("close barq") {
        barq_path path;
        barq::native::db_config config;
        config.set_path(path);
        auto barq = db(std::move(config));

        AllTypesObject obj;
        obj.str_col = "John";
        obj._id = 1;
        AllTypesObjectLink obj_link;
        obj_link.str_col = "Fido";
        obj.opt_obj_col = &obj_link;

        auto managed_obj = barq.write([&barq, &obj] {
            return barq.add(std::move(obj));
        });

        barq.close();
        CHECK(managed_obj.is_invalidated());
        CHECK(barq.is_closed());
    }

    TEST_CASE("compact on launch") {
        barq_path path;

        auto config = barq::native::db_config();
        config.set_path(path);
        size_t compact_run_count = 0;
        config.should_compact_on_launch([&compact_run_count](uint64_t total_bytes, uint64_t used_bytes) {
            compact_run_count++;
            return true;
        });
        {
            auto barq = db(config);
            barq.write([&]() {
                for (int64_t i = 0; i < 200; ++i) {
                    AllTypesObject o;
                    o._id = i;
                    barq.add(std::move(o));
                }
            });
        }
        auto file_size_before = std::filesystem::file_size(path.path);
        CHECK(compact_run_count == 1);
        std::thread([&] {
            auto other_barq = db(config);
        }).join();

        auto file_size_after = std::filesystem::file_size(path.path);
        CHECK(compact_run_count == 2);
        CHECK(file_size_before > file_size_after);
    }
}