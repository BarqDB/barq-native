#include "../main.hpp"
#include "test_objects.hpp"

namespace barq::native {
    struct results_wrapper {
        explicit results_wrapper(const db_config &c) {
            m_barq = std::make_unique<db>(c);
        }
        auto get_results() const {
            return m_barq->objects<AllTypesObject>()
                    .where([&](auto &data) { return data.double_col != 5 && data.str_col == "foo"; });
        }

        void observe() {
            m_token = get_results().observe([&](auto &&c) {
                run_count++;
                barq::native::results<barq::native::AllTypesObject> *res = c.collection;
                CHECK_FALSE(res == nullptr);
            });
        }

        void unregister() {
            m_token.unregister();
        }

        size_t run_count = 0;
    private:
        std::unique_ptr<db> m_barq;
        barq::native::notification_token m_token;
    };

    TEST_CASE("results", "[results]") {
        barq_path path;
        barq::native::db_config config;
        config.set_path(path);
        SECTION("results_notifications") {
            auto barq = db(std::move(config));

            AllTypesObject obj;
            obj.str_col = "John";
            obj._id = 1;
            barq.write([&barq, &obj]() {
                barq.add(std::move(obj));
            });
            auto results = barq.objects<AllTypesObject>();

            bool did_run = false;

            barq::native::results<AllTypesObject>::results_change change;

            auto require_change = [&] {
                auto token = results.observe([&](auto&& c) {
                    did_run = true;
                    change = std::move(c);
                    CHECK_FALSE(c.collection == nullptr);
                });
                return token;
            };

            auto token = require_change();
            AllTypesObject obj2;
            obj2.str_col = "Jane";
            obj2._id = 2;
            barq.write([&barq, &obj2]() {
                barq.add(std::move(obj2));
            });
            barq.refresh();

            CHECK(change.insertions.size() == 1);
            CHECK(change.collection->size() == 2);
            CHECK(did_run);
        }

        SECTION("results_notifications_insertions") {
            auto barq = db(std::move(config));
            barq.write([&barq] {
                AllTypesObject o;
                o._id = 1;
                barq.add(std::move(o));
            });

            bool did_run = false;

            results<AllTypesObject>::results_change change;

            int callback_count = 0;
            auto results = barq.objects<AllTypesObject>();
            auto require_change = [&] {
                auto token = results.observe([&](auto&& c) {
                    CHECK(c.collection == &results);
                    callback_count++;
                    change = std::move(c);
                });
                return token;
            };

            auto token = require_change();
            barq.write([&barq] {
                AllTypesObject o;
                o._id = 2;
                barq.add(std::move(o));
            });

            barq.write([] {});

            CHECK(change.insertions.size() == 1);
            CHECK(change.deletions.size() == 0);
            CHECK(change.modifications.size() == 0);

            barq.write([&barq] {
                AllTypesObject o1;
                o1._id = 3;
                barq.add(std::move(o1));
                AllTypesObject o2;
                o2._id = 4;
                barq.add(std::move(o2));
            });

            barq.write([] {});

            CHECK(change.insertions.size() == 2);
            CHECK(change.deletions.empty());
            CHECK(change.modifications.empty());
            CHECK(callback_count == 3);
        }

        SECTION("results_notifications_deletions") {
            auto obj = AllTypesObject();

            auto barq = db(std::move(config));
            auto managed_obj = barq.write([&barq, &obj] {
                return barq.add(std::move(obj));
            });

            bool did_run = false;
            results<AllTypesObject>::results_change change;
            auto results = barq.objects<AllTypesObject>();

            auto require_change = [&] {
                auto token = results.observe([&](auto&& c) {
                    CHECK(c.collection == &results);
                    did_run = true;
                    change = std::move(c);
                });
                return token;
            };

            auto token = require_change();
            barq.write([&barq, &managed_obj] {
                barq.remove(managed_obj);
            });
            barq.refresh();
            CHECK(change.deletions.size() == 1);
            CHECK(change.insertions.size() == 0);
            CHECK(change.modifications.size() == 0);
            CHECK(did_run);
        }

        SECTION("results_notifications_modifications") {
            auto obj = AllTypesObject();

            auto barq = db(std::move(config));
            auto managed_obj = barq.write([&barq, &obj] {
                return barq.add(std::move(obj));
            });

            bool did_run = false;

            results<AllTypesObject>::results_change change;
            auto results = barq.objects<AllTypesObject>();

            auto require_change = [&] {
                auto token = results.observe([&](auto&& c) {
                    CHECK(c.collection == &results);
                    did_run = true;
                    change = std::move(c);
                });
                return token;
            };

            auto token = require_change();
            barq.write([&barq, &managed_obj] {
                managed_obj.str_col = "foobar";
            });
            barq.refresh();
            CHECK(change.modifications.size() == 1);
            CHECK(change.insertions.size() == 0);
            CHECK(change.deletions.size() == 0);
            CHECK(did_run);
        }

        managed<AllTypesObject> test_obj;

        SECTION("results_subscript") {
            auto barq = db(std::move(config));

            AllTypesObject obj;
            obj.str_col = "John";
            obj._id = 1;
            obj.double_col = 1;
            auto managed_obj = barq.write([&barq, &obj]() {
                return barq.add(std::move(obj));
            });
            auto results = barq.objects<AllTypesObject>();
            {
                auto results1 = barq.objects<AllTypesObject>();
                test_obj = results1[0];
            }
            auto o = results[0];
            CHECK(o.str_col == "John");
            CHECK(o.list_obj_col.size() == 0);
            auto copy_o = o;
            CHECK(copy_o.str_col == "John");
            CHECK(copy_o.list_obj_col.size() == 0);
            auto moved_o = std::move(o);
            CHECK(moved_o.str_col == "John");
            CHECK(moved_o.list_obj_col.size() == 0);

            barq.write([&](){
                test_obj.double_col = 123.456;
            });
            CHECK(copy_o.double_col == 123.456);
            CHECK(moved_o.double_col == 123.456);
        }

        SECTION("results_iterator_object") {
            auto barq = db(std::move(config));

            AllTypesObject obj;
            obj.str_col = "foo";
            obj._id = 1;

            AllTypesObject obj2;
            obj2.str_col = "bar";
            obj2._id = 2;

            barq.write([&barq, &obj, &obj2]() {
                barq.add(std::move(obj));
                barq.add(std::move(obj2));

            });
            auto results = barq.objects<AllTypesObject>();

            managed<AllTypesObject> obj_from_loop;
            size_t count = 0;
            for (auto o : results) {
                count++;
                if (count == 1) {
                    CHECK(o._id == 1);
                    CHECK(o.str_col == "foo");
                    obj_from_loop = o;
                } else {
                    CHECK(o._id == 2);
                    CHECK(o.str_col == "bar");
                }
            }
            CHECK(obj_from_loop.str_col == "foo");
            CHECK(count == 2);
        }

        SECTION("results_iterator_primitive") {
            auto barq = db(std::move(config));

            AllTypesObject obj;
            obj.list_int_col = {1, 2, 3};

            auto managed_obj = barq.write([&]() {
                return barq.add(std::move(obj));
            });
            auto results = managed_obj.list_int_col.as_results();

            std::vector<int64_t> res;
            for (auto o : results) {
                res.push_back(o);
            }
            CHECK(res == std::vector<int64_t>({1, 2, 3}));
        }

        SECTION("results_iterator_mixed") {
            auto barq = db(std::move(config));

            AllTypesObject obj;
            obj.list_mixed_col = {barq::native::mixed((int64_t)1), barq::native::mixed((int64_t)2), barq::native::mixed((int64_t)3)};

            auto managed_obj = barq.write([&]() {
                return barq.add(std::move(obj));
            });
            auto results = managed_obj.list_mixed_col.as_results();

            std::vector<barq::native::mixed> res;
            for (auto o : results) {
                res.push_back(o);
            }
            CHECK(res == std::vector<barq::native::mixed>({barq::native::mixed((int64_t)1), barq::native::mixed((int64_t)2), barq::native::mixed((int64_t)3)}));
        }

        SECTION("results_iterator_enum") {
            auto barq = db(std::move(config));

            AllTypesObject obj;
            obj.list_enum_col = {AllTypesObject::Enum::one, AllTypesObject::Enum::two};

            auto managed_obj = barq.write([&]() {
                return barq.add(std::move(obj));
            });
            auto results = managed_obj.list_enum_col.as_results();

            std::vector<AllTypesObject::Enum> res;
            for (auto o : results) {
                res.push_back(o);
            }
            CHECK(res == std::vector<AllTypesObject::Enum>({AllTypesObject::Enum::one, AllTypesObject::Enum::two}));
        }

        SECTION("results_query") {
            auto barq = db(std::move(config));

            AllTypesObject obj;
            obj.str_col = "foo";
            obj._id = 1;

            AllTypesObject obj2;
            obj2.str_col = "bar";
            obj2._id = 2;

            barq.write([&barq, &obj, &obj2]() {
                barq.add(std::move(obj));
                barq.add(std::move(obj2));

            });
            auto results = barq.objects<AllTypesObject>();
            auto queried_results = results.where([](auto& o) {
                return o._id == 1;
            });
            CHECK(queried_results.size() == 1);
            CHECK(queried_results[0]._id == 1);

            auto queried_results_string = results.where("_id == $0", {(int64_t)2});
            CHECK(queried_results_string.size() == 1);
            CHECK(queried_results_string[0]._id == 2);

            queried_results_string = results.where("_id == $0", {(int64_t)3});
            CHECK(queried_results_string.size() == 0);
        }

        SECTION("results_sort") {
            auto barq = db(std::move(config));

            AllTypesObject obj;
            obj.str_col = "foo";
            obj._id = 1;

            AllTypesObject obj2;
            obj2.str_col = "bar";
            obj2._id = 2;

            barq.write([&barq, &obj, &obj2]() {
                barq.add(std::move(obj));
                barq.add(std::move(obj2));
            });
            auto sorted_results_ascending = barq.objects<AllTypesObject>().sort("str_col", true);
            CHECK(sorted_results_ascending[0].str_col == "bar");
            CHECK(sorted_results_ascending[1].str_col == "foo");
            auto sorted_results_descending = barq.objects<AllTypesObject>().sort("str_col", false);
            CHECK(sorted_results_descending[0].str_col == "foo");
            CHECK(sorted_results_descending[1].str_col == "bar");

            auto sorted_results_with_descriptors_ascending = barq.objects<AllTypesObject>().sort({{"str_col", true}, {"_id", true}});
            CHECK(sorted_results_with_descriptors_ascending[0].str_col == "bar");
            CHECK(sorted_results_with_descriptors_ascending[1].str_col == "foo");
            auto sorted_results_with_descriptors_descending = barq.objects<AllTypesObject>().sort({{"str_col", false}, {"_id", false}});
            CHECK(sorted_results_with_descriptors_descending[0].str_col == "foo");
            CHECK(sorted_results_with_descriptors_descending[1].str_col == "bar");
        }

        SECTION("observe_results_derived_from_list") {
            auto barq = db(std::move(config));

            AllTypesObjectLink link;
            link.str_col = "bar";
            link._id = 1;

            AllTypesObject obj;
            obj.str_col = "foo";
            obj._id = 1;
            obj.list_obj_col.push_back(&link);

            barq.write([&barq, &obj]() {
                return barq.add(std::move(obj));
            });

            auto res = barq.objects<AllTypesObject>()[0].list_obj_col.where([](auto&& o) {
                return o._id == 1;
            });

            int notification_count = 0;
            auto token = res.observe([&notification_count](auto&& change) {
                notification_count++;
            });

            barq.write([&barq, &res]() {
                res[0].str_col = "foo";
            });
            barq.refresh();

            barq.write([&barq, &res]() {
                res[0].str_col = "bar";
            });
            barq.refresh();

            CHECK(notification_count == 3);
            token.unregister();
            barq.write([&barq, &res]() {
                res[0].str_col = "abc";
            });
            barq.refresh();
            CHECK(notification_count == 3);
        }

        SECTION("observe_results_in_wrapper") {
            auto barq = db(config);

            AllTypesObjectLink link;
            link.str_col = "bar";
            link._id = 1;

            AllTypesObject obj;
            obj.str_col = "foo";
            obj._id = 1;
            obj.list_obj_col.push_back(&link);

            barq.write([&barq, &obj]() {
                return barq.add(std::move(obj));
            });

            results_wrapper wrapper(config);
            wrapper.observe();
            auto res = wrapper.get_results();

            barq.write([&barq, &res]() {
                res[0].str_col = "foo";
            });
            barq.refresh();

            barq.write([&barq, &res]() {
                res[0].str_col = "bar";
            });
            barq.refresh();

            CHECK(wrapper.run_count == 3);
            wrapper.unregister();
            barq.write([&barq, &res]() {
                AllTypesObject o;
                o.str_col = "foo";
                o._id = 1;
                barq.add(std::move(o));
            });
            barq.refresh();
            CHECK(wrapper.run_count == 3);
        }
    }
}