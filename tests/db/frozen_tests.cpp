#include "../main.hpp"
#include "test_objects.hpp"

namespace barq::native {
    TEST_CASE("frozen_barq") {
        barq_path path;
        barq::native::db_config config;
        config.set_path(path);

        SECTION("is_frozen") {
            auto barq = db(std::move(config));
            auto frozen_barq = barq.freeze();
            CHECK(frozen_barq.is_frozen());

            auto frozen_barq_copy = frozen_barq;
            CHECK(frozen_barq_copy.is_frozen());
        }

        SECTION("refresh_frozen") {
            auto barq = db(std::move(config));
            auto frozen_barq = db(barq.freeze());
            CHECK(frozen_barq.is_frozen());

            barq.write([&barq]() {
                barq.add(AllTypesObject());
            });

            CHECK_FALSE(frozen_barq.refresh());
            CHECK(frozen_barq.objects<AllTypesObject>().size() == 0);
        }

        SECTION("forbidden_methods") {
            auto barq = db(std::move(config));
            auto frozen_barq = db(barq.freeze());
            CHECK(frozen_barq.is_frozen());

            CHECK_THROWS_WITH(frozen_barq.begin_write(), "Can't perform transactions on a frozen Barq");
            CHECK_THROWS_WITH(frozen_barq.objects<AllTypesObject>().observe([](auto) { }), "Notifications are not available on frozen collections since they do not change.");
        }

        SECTION("object_is_frozen_from_results") {
            auto barq = db(std::move(config));
            barq.write([&barq]() {
                barq.add(AllTypesObject());
            });
            auto frozen_barq = db(barq.freeze());
            frozen_barq.invalidate();
            auto managed_obj = frozen_barq.objects<AllTypesObject>()[0];
            CHECK(managed_obj.is_frozen());
        }

        SECTION("thaw") {
            auto barq = db(std::move(config));
            barq.write([&barq]() {
                barq.add(AllTypesObject());
            });
            auto frozen_barq = barq.freeze();
            CHECK(frozen_barq.is_frozen());

            auto thawed = frozen_barq.thaw();
            CHECK_FALSE(thawed.is_frozen());
            thawed.write([&thawed]() {
                auto o = AllTypesObject();
                o._id = 2;
                thawed.add(std::move(o));
            });
            thawed.write([&thawed]() {
                auto o = AllTypesObject();
                o._id = 3;
                thawed.add(std::move(o));
            });
            CHECK_FALSE(thawed.refresh());
            CHECK(thawed.objects<AllTypesObject>().size() == 3);

            CHECK(frozen_barq.is_frozen());
            CHECK_FALSE(frozen_barq.refresh());
            CHECK(frozen_barq.objects<AllTypesObject>().size() == 1);
        }

        SECTION("thaw_different_thread") {
            auto barq = db(std::move(config));
            barq.write([&barq]() {
                barq.add(AllTypesObject());
            });
            auto frozen_barq = barq.freeze();
            CHECK(frozen_barq.is_frozen());

            auto async = std::async(std::launch::async, [&frozen_barq](){
                auto thawed = frozen_barq.thaw();
                CHECK_FALSE(thawed.is_frozen());
                thawed.write([&thawed]() {
                    auto o = AllTypesObject();
                    o._id = 2;
                    thawed.add(std::move(o));
                });
                thawed.write([&thawed]() {
                    auto o = AllTypesObject();
                    o._id = 3;
                    thawed.add(std::move(o));
                });
                CHECK_FALSE(thawed.refresh());
                CHECK(thawed.objects<AllTypesObject>().size() == 3);

                CHECK(frozen_barq.is_frozen());
                CHECK_FALSE(frozen_barq.refresh());
                CHECK(frozen_barq.objects<AllTypesObject>().size() == 1);
                return true;
            });
            CHECK(async.get());
        }

        SECTION("thaw_previous_version") {
            auto barq = db(std::move(config));
            auto frozen_barq = barq.freeze();
            CHECK(frozen_barq.is_frozen());
            barq.write([&barq]() {
                barq.add(AllTypesObject());
            });
            CHECK(frozen_barq.objects<AllTypesObject>().size() == 0);

            auto thawed = frozen_barq.thaw();
            CHECK_FALSE(thawed.is_frozen());
            CHECK(thawed.objects<AllTypesObject>().size() == 1);
        }

        SECTION("freeze_same_barq_returns_self") {
            db barq = db(std::move(config));
            db frozen_barq1 = barq.freeze();
            db frozen_barq2 = barq.freeze();
            db frozen_barq3 = frozen_barq2.freeze();

            CHECK(frozen_barq1 == frozen_barq2);
            CHECK(frozen_barq2 == frozen_barq3);
            CHECK_FALSE(barq == frozen_barq1);
        }

        SECTION("frozen_object") {
            auto barq = db(std::move(config));
            auto object = barq.write([&barq]() {
                return barq.add(AllTypesObject());
            });

            auto frozen_object = object.freeze();
            CHECK(frozen_object.is_frozen());
            auto same_frozen_object = frozen_object.freeze();
            CHECK(frozen_object == same_frozen_object);
            CHECK_FALSE(frozen_object == object);
        }

        SECTION("frozen_object_different_thread") {
            auto barq = db(std::move(config));
            auto object = barq.write([&barq]() {
                auto o = AllTypesObject();
                o.str_col = "bar";
                o._id = 1;

                auto link = AllTypesObjectLink();
                link._id = 1;
                link.str_col = "foobar";

                o.opt_obj_col = &link;
                o.list_obj_col.push_back(&link);
                o.set_obj_col.insert(&link);
                o.map_link_col["foo"] = &link;

                return barq.add(std::move(o));
            });

            auto frozen_object = object.freeze();
            CHECK(frozen_object.is_frozen());

            auto async = std::async(std::launch::async, [&frozen_object]() {
                CHECK(frozen_object.is_frozen());
                CHECK(frozen_object._id == 1);
                CHECK(frozen_object.str_col == "bar");
                CHECK(frozen_object.opt_obj_col->_id == 1);
                CHECK(frozen_object.opt_obj_col->str_col == "foobar");
                CHECK(frozen_object.list_obj_col[0]->str_col == "foobar");
                CHECK(frozen_object.map_link_col["foo"]->str_col == "foobar");
                CHECK((*frozen_object.set_obj_col.find(frozen_object.opt_obj_col)).str_col == "foobar");

                return true;
            });
            CHECK(async.get());
        }

        SECTION("frozen_object_observe") {
            auto barq = db(std::move(config));
            auto object = barq.write([&barq]() {
                auto o = AllTypesObject();
                o._id = 1;
                return barq.add(std::move(o));
            });

            auto frozen_object = object.freeze();
            CHECK(frozen_object.is_frozen());

            CHECK_THROWS_WITH(frozen_object.observe([](auto) { }), "Notifications are not available on frozen collections since they do not change.");
            CHECK_THROWS_WITH(frozen_object.list_int_col.observe([](auto) { }), "Notifications are not available on frozen collections since they do not change.");
            CHECK_THROWS_WITH(frozen_object.list_obj_col.observe([](auto) { }), "Notifications are not available on frozen collections since they do not change.");
            CHECK_THROWS_WITH(frozen_object.set_int_col.observe([](auto) { }), "Notifications are not available on frozen collections since they do not change.");
            CHECK_THROWS_WITH(frozen_object.set_obj_col.observe([](auto) { }), "Notifications are not available on frozen collections since they do not change.");
            CHECK_THROWS_WITH(frozen_object.map_int_col.observe([](auto) { }), "Notifications are not available on frozen collections since they do not change.");
            CHECK_THROWS_WITH(frozen_object.map_link_col.observe([](auto) { }), "Notifications are not available on frozen collections since they do not change.");
        }

        SECTION("frozen_object_comparison") {
            auto barq = db(std::move(config));
            auto object = barq.write([&barq]() {
                auto o = AllTypesObject();
                o._id = 1;
                return barq.add(std::move(o));
            });

            auto frozen_object = object.freeze();
            CHECK(frozen_object.is_frozen());
            auto frozen_object2 = object.freeze();
            CHECK(frozen_object2.is_frozen());

            CHECK(frozen_object == frozen_object2);
            CHECK_FALSE(frozen_object != frozen_object2);
            CHECK(object != frozen_object2);
            CHECK_FALSE(object == frozen_object2);
        }

        SECTION("frozen_object_mutate") {
            auto barq = db(std::move(config));
            auto object = barq.write([&barq]() {
                auto o = AllTypesObject();
                o._id = 1;
                return barq.add(std::move(o));
            });

            auto frozen_object = object.freeze();
            barq.write([&barq, &object]() {
                object.str_col = "doesn't throw exception";
            });

            CHECK_THROWS_WITH(
                barq.write([&barq, &frozen_object]() {
                    frozen_object.str_col = "throws exception";
                }), "Trying to modify database while in read transaction"
            );
        }

        SECTION("frozen_object_linking_objects") {
            auto barq = db(std::move(config));

            barq.write([&barq]() {
                Dog dog;
                dog._id = 1;
                dog.name = "Spot";

                Person person;
                person.name = "John";
                person._id = 1;
                person.dog = &dog;
                barq.add(std::move(person));
            });

            auto dog = barq.objects<Dog>().freeze()[0];
            auto frozen_object = dog;
            auto owner = frozen_object.owners[0];
            CHECK(owner.is_frozen());
            CHECK(owner.name == "John");
        }

        SECTION("frozen_thawed_results") {
            auto barq = db(std::move(config));

            barq.write([&barq]() {
                Dog dog;
                dog._id = 1;
                dog.name = "Spot";

                Person person;
                person.name = "John";
                person._id = 1;
                person.dog = &dog;
                barq.add(std::move(person));
            });

            auto frozen_results = barq.objects<Dog>().freeze();
            auto dog = frozen_results[0];
            CHECK(dog.is_frozen());
            CHECK(frozen_results.is_frozen());
            CHECK_THROWS_WITH(frozen_results.observe([](auto) { }), "Notifications are not available on frozen collections since they do not change.");
            auto thawed_results = frozen_results.thaw();
            CHECK_FALSE(thawed_results.is_frozen());
            dog = thawed_results[0];
            CHECK_FALSE(dog.is_frozen());
        }

        SECTION("frozen_object_thaw") {
            auto barq = db(std::move(config));

            auto managed_obj = barq.write([&barq]() {
                Dog dog;
                dog._id = 1;
                dog.name = "Spot";

                Person person;
                person.name = "John";
                person._id = 1;
                person.dog = &dog;
                return barq.add(std::move(person));
            });

            auto frozen_obj = managed_obj.freeze();
            CHECK(frozen_obj.is_frozen());

            auto thawed_obj =  frozen_obj.thaw();
            CHECK_FALSE(thawed_obj.is_frozen());
            auto thawed_barq = thawed_obj.get_barq();

            CHECK_FALSE(thawed_barq.is_frozen());

            thawed_barq.write([&thawed_obj]() {
                thawed_obj.name = "Bob";
            });

            barq.refresh();

            CHECK(managed_obj.name == "Bob");
        }


        SECTION("reuse_frozen_barq_from_thaw") {
            auto barq = db(std::move(config));

            auto managed_obj = barq.write([&barq]() {
                return barq.add(AllTypesObject());
            });
            auto frozen_barq = barq.freeze();
            CHECK(frozen_barq.is_frozen());

            auto thawed = frozen_barq.thaw();
            CHECK_FALSE(thawed.is_frozen());
            thawed.write([&]() {
                managed_obj.str_col = "bob";
            });
            CHECK(managed_obj.str_col == "bob");
        }
    }
}