#include "../main.hpp"
#include "test_objects.hpp"

using namespace barq::native;

template<typename T, typename Func>
void test(barq::native::managed<T>* property, Func f, 
          std::vector<typename T::value_type>&& values, size_t&& expected_count = 3) {
    f(property, values, expected_count);
}

template<auto UnmanagedPtr, auto ManagedPtr, typename Func, typename T>
void test(Func f, std::vector<T>&& values, size_t&& expected_count = 3) {
    constexpr auto unmanaged_ptr = UnmanagedPtr;
    constexpr auto managed_ptr = ManagedPtr;
    f(unmanaged_ptr, managed_ptr, values, expected_count);
}

TEST_CASE("set", "[set]") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);

    SECTION("insert") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();

        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        auto scenario = [&](auto& p, auto& values, size_t expected_count) {
            barq.write([&]() {
                for (auto v: values) {
                    p->insert(v);
                }
            });
            CHECK(p->size() == expected_count);
        };

        test(&managed_obj.set_int_col, scenario, {42, 42, 24, -1});
        test(&managed_obj.set_double_col, scenario, {42.001, 42.001, 24.001, -1.001});
        test(&managed_obj.set_bool_col, scenario, {true, false, true}, 2);
        test(&managed_obj.set_str_col, scenario, {"42", "42", "24", "-1"});
        test(&managed_obj.set_uuid_col, scenario, {barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"), 
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120001"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002")});
        auto obj_id = barq::native::object_id::generate();
        test(&managed_obj.set_object_id_col, scenario, {obj_id, obj_id, barq::native::object_id::generate(), barq::native::object_id::generate()});

        auto bin_data = std::vector<uint8_t>({1, 2, 3, 4});
        test(&managed_obj.set_binary_col, scenario, {bin_data, bin_data, std::vector<uint8_t>({1, 3, 4}), std::vector<uint8_t>({1})});

        auto time = std::chrono::system_clock::now();
        auto time2 = time + time.time_since_epoch();
        test(&managed_obj.set_date_col, scenario, {time, time, time2, std::chrono::time_point<std::chrono::system_clock>()});
        test(&managed_obj.set_mixed_col, scenario, { barq::native::mixed((int64_t)42), barq::native::mixed((int64_t)42), barq::native::mixed("24"), barq::native::mixed(barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002"))});
    }

    SECTION("find") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        auto scenario = [&](auto& p, auto& values, size_t expected_count) {
            barq.write([&]() {
                p->insert(values[0]);
            });
            CHECK(p->size() == 1);

            auto it = p->find(values[0]);
            CHECK(it != p->end());
            CHECK(*it == values[0]);
            auto it2 = p->find(values[values.size() - 1]);
            CHECK(it2 == p->end());
        };

        test(&managed_obj.set_int_col, scenario, {42, 42, 24, -1});
        test(&managed_obj.set_double_col, scenario, {42.001, 42.001, 24.001, -1.001});
        test(&managed_obj.set_str_col, scenario, {"42", "42", "24", "-1"});
        test(&managed_obj.set_uuid_col, scenario, {barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120001"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002")});
        auto obj_id = barq::native::object_id::generate();
        test(&managed_obj.set_object_id_col, scenario, {obj_id, obj_id, barq::native::object_id::generate(), barq::native::object_id::generate()});

        auto bin_data = std::vector<uint8_t>({1, 2, 3, 4});
        test(&managed_obj.set_binary_col, scenario, {bin_data, bin_data, std::vector<uint8_t>({1, 3, 4}), std::vector<uint8_t>({1})});

        auto time = std::chrono::system_clock::now();
        auto time2 = time + time.time_since_epoch();
        test(&managed_obj.set_date_col, scenario, {time, time, time2, std::chrono::time_point<std::chrono::system_clock>()});
        test(&managed_obj.set_mixed_col, scenario, { barq::native::mixed((int64_t)42), barq::native::mixed((int64_t)42), barq::native::mixed("24"), barq::native::mixed(barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002"))});

        barq.write([&]() {
            managed_obj.set_bool_col.insert(true);
        });
        CHECK(managed_obj.set_bool_col.size() == 1);

        auto it = managed_obj.set_bool_col.find(true);
        CHECK(it != managed_obj.set_bool_col.end());
        CHECK(*it == true);
        auto it2 = managed_obj.set_bool_col.find(false);
        CHECK(it2 == managed_obj.set_bool_col.end());
    }

    SECTION("erase") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        auto scenario = [&](auto& p, auto& values, size_t expected_count) {
            barq.write([&]() {
                for (auto& v : values) {
                    p->insert(v);
                }
            });

            auto it = p->find(values[0]);
            barq.write([&]() {
                p->erase(it);
            });
            it =  p->find(values[0]);
            CHECK(it ==  p->end());
            CHECK(p->size() == 2);
            barq.write([&]() {
                p->clear();
            });
            CHECK(p->size() == 0);
        };

        test(&managed_obj.set_int_col, scenario, {42, 42, 24, -1});
        test(&managed_obj.set_double_col, scenario, {42.001, 42.001, 24.001, -1.001});
        test(&managed_obj.set_str_col, scenario, {"42", "42", "24", "-1"});
        test(&managed_obj.set_uuid_col, scenario, {barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120001"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002")});
        auto obj_id = barq::native::object_id::generate();
        test(&managed_obj.set_object_id_col, scenario, {obj_id, obj_id, barq::native::object_id::generate(), barq::native::object_id::generate()});

        auto bin_data = std::vector<uint8_t>({1, 2, 3, 4});
        test(&managed_obj.set_binary_col, scenario, {bin_data, bin_data, std::vector<uint8_t>({1, 3, 4}), std::vector<uint8_t>({1})});

        auto time = std::chrono::system_clock::now();
        auto time2 = time + time.time_since_epoch();
        test(&managed_obj.set_date_col, scenario, {time, time, time2, std::chrono::time_point<std::chrono::system_clock>()});
        test(&managed_obj.set_mixed_col, scenario, { barq::native::mixed((int64_t)42), barq::native::mixed((int64_t)42), barq::native::mixed("24"), barq::native::mixed(barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002"))});

        barq.write([&]() {
            managed_obj.set_bool_col.insert(true);
            managed_obj.set_bool_col.insert(false);
        });

        auto it = managed_obj.set_bool_col.find(true);
        barq.write([&]() {
            managed_obj.set_bool_col.erase(it);
        });
        it =  managed_obj.set_bool_col.find(true);
        CHECK(it ==  managed_obj.set_bool_col.end());
        CHECK(managed_obj.set_bool_col.size() == 1);
        barq.write([&]() {
            managed_obj.set_bool_col.clear();
        });
        CHECK(managed_obj.set_bool_col.size() == 0);
    }

    SECTION("notifications") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        auto scenario = [&](auto& p, auto& values, size_t expected_count) {
            size_t callback_count = 0;
            auto token = p->observe([&](auto&& c) {
                callback_count++;
                if (callback_count == 2)
                    CHECK(c.insertions.size() == 3);
                else if (callback_count == 3)
                    CHECK(c.deletions.size() == 3);
            });
            barq.write([&]() {
                for (auto& v : values) {
                    p->insert(v);
                }
            });
            barq.refresh();
            CHECK(callback_count == 2);

            barq.write([&]() {
                p->clear();
            });
            barq.refresh();
            CHECK(callback_count == 3);
        };

        test(&managed_obj.set_int_col, scenario, {42, 42, 24, -1});
        test(&managed_obj.set_double_col, scenario, {42.001, 42.001, 24.001, -1.001});
        test(&managed_obj.set_str_col, scenario, {"42", "42", "24", "-1"});
        test(&managed_obj.set_uuid_col, scenario, {barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120001"),
                                                   barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002")});
        auto obj_id = barq::native::object_id::generate();
        test(&managed_obj.set_object_id_col, scenario, {obj_id, obj_id, barq::native::object_id::generate(), barq::native::object_id::generate()});

        auto bin_data = std::vector<uint8_t>({1, 2, 3, 4});
        test(&managed_obj.set_binary_col, scenario, {bin_data, bin_data, std::vector<uint8_t>({1, 3, 4}), std::vector<uint8_t>({1})});

        auto time = std::chrono::system_clock::now();
        auto time2 = time + time.time_since_epoch();
        test(&managed_obj.set_date_col, scenario, {time, time, time2, std::chrono::time_point<std::chrono::system_clock>()});
        test(&managed_obj.set_mixed_col, scenario, { barq::native::mixed((int64_t)42), barq::native::mixed((int64_t)42), barq::native::mixed("24"), barq::native::mixed(barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002"))});

        size_t callback_count = 0;
        auto token = managed_obj.set_bool_col.observe([&](auto&& c) {
            callback_count++;
            if (callback_count == 2)
                CHECK(c.insertions.size() == 2);
            else if (callback_count == 3)
                CHECK(c.deletions.size() == 2);
        });
        barq.write([&]() {
            managed_obj.set_bool_col.insert(true);
            managed_obj.set_bool_col.insert(false);
        });
        barq.refresh();
        CHECK(callback_count == 2);

        barq.write([&]() {
            managed_obj.set_bool_col.clear();
        });
        barq.refresh();
        CHECK(callback_count == 3);
    }

    SECTION("detach()") {

        auto scenario = [&](auto unmanaged_ptr, auto managed_ptr, auto& values, size_t expected_count) {
            auto barq = barq::native::db(config);
            auto obj = barq::native::AllTypesObject();

            for (auto& v : values) {
                (obj.*unmanaged_ptr).insert(v);
            }

            auto managed_obj = barq.write([&]() {
                return barq.add(std::move(obj));
            });

            auto other = (managed_obj.*managed_ptr).detach();
            using ElementType = typename std::remove_reference<decltype(*values.begin())>::type;
            CHECK(other == std::set<ElementType>(values.begin(), values.end()));
        };

        test<&AllTypesObject::set_int_col,
             &managed<AllTypesObject>::set_int_col>(scenario,
                                                                                std::vector<int64_t>({1, 2, 3, 3}));
        test<&AllTypesObject::set_double_col,
             &managed<AllTypesObject>::set_double_col>(scenario,
                                                                                   std::vector<double>({42.001, 42.001, 24.001, -1.001}));
        test<&AllTypesObject::set_str_col,
             &managed<AllTypesObject>::set_str_col>(scenario,
                                                                                std::vector<std::string>({"42", "42", "24", "-1"}));
        test<&AllTypesObject::set_uuid_col,
             &managed<AllTypesObject>::set_uuid_col>(scenario,
                                                                                std::vector<barq::native::uuid>({barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"),
                                                                                                          barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120000"),
                                                                                                          barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120001"),
                                                                                                          barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002")}));
        auto obj_id = barq::native::object_id::generate();
        test<&AllTypesObject::set_object_id_col,
             &managed<AllTypesObject>::set_object_id_col>(scenario,
                                                                                      std::vector<barq::native::object_id>({obj_id,
                                                                                                                     obj_id,
                                                                                                                     barq::native::object_id::generate(),
                                                                                                                     barq::native::object_id::generate()}));
        auto bin_data = std::vector<uint8_t>({1, 2, 3, 4});
        test<&AllTypesObject::set_binary_col,
             &managed<AllTypesObject>::set_binary_col>(scenario,
                                                                                   std::vector<std::vector<uint8_t>>({bin_data,
                                                                                   bin_data,
                                                                                   std::vector<uint8_t>({1, 3, 4}),
                                                                                   std::vector<uint8_t>({1})}));

        auto time = std::chrono::system_clock::now();
        auto time2 = time + time.time_since_epoch();
        test<&AllTypesObject::set_date_col,
             &managed<AllTypesObject>::set_date_col>(scenario,
                                                                                 std::vector<std::chrono::time_point<std::chrono::system_clock>>({time,
                                                                                                                    time,
                                                                                                                    time2,
                                                                                                                    std::chrono::time_point<std::chrono::system_clock>()}));


        test<&AllTypesObject::set_mixed_col,
             &managed<AllTypesObject>::set_mixed_col>(scenario,
                                                                                  std::vector<barq::native::mixed>({
                 barq::native::mixed((int64_t)42),
                 barq::native::mixed((int64_t)42),
                 barq::native::mixed("24"),
                 barq::native::mixed(barq::native::uuid("18de7916-7f84-11ec-a8a3-0242ac120002"))}));
    }

    SECTION("set_algorithms") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        obj.set_int_col.insert(3);

        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        std::set<int64_t> other = {1,2,3};
        std::set<int64_t> res;
        std::set_intersection(managed_obj.set_int_col.begin(), managed_obj.set_int_col.end(), other.begin(), other.end(), std::inserter(res, res.begin()));
        CHECK(res == std::set<int64_t>({3}));

        barq.write([&]() {
            managed_obj.set_int_col.clear();
        });
        std::set<int64_t> a = {1, 2, 3};
        std::set<int64_t> b = {2, 3};

        barq.write([&]() {
            std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(managed_obj.set_int_col, managed_obj.set_int_col.begin()));
        });
        CHECK(managed_obj.set_int_col.detach() == std::set<int64_t>({2, 3}));

        barq.write([&]() {
            managed_obj.set_int_col.clear();
            std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::inserter(managed_obj.set_int_col, managed_obj.set_int_col.begin()));
        });
        CHECK(managed_obj.set_int_col.detach() == std::set<int64_t>({1, 2, 3}));


        barq.write([&]() {
            managed_obj.set_int_col.clear();
            std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(managed_obj.set_int_col, managed_obj.set_int_col.begin()));
        });
        CHECK(managed_obj.set_int_col.detach() == std::set<int64_t>({1}));

        barq.write([&]() {
            managed_obj.set_int_col.clear();
            std::set_symmetric_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(managed_obj.set_int_col, managed_obj.set_int_col.begin()));
        });
        CHECK(managed_obj.set_int_col.detach() == std::set<int64_t>({1}));
    }

    SECTION("iterator") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });
        barq.write([&]() {
            managed_obj.set_int_col.insert(1);
            managed_obj.set_int_col.insert(2);
            managed_obj.set_int_col.insert(3);
            managed_obj.set_int_col.insert(3);
        });

        std::set<int64_t> res;
        for (const auto& x : managed_obj.set_int_col) {
            res.insert(x);
        }
        CHECK(res == std::set<int64_t>({1, 2, 3}));
        res.clear();

        for (auto it = managed_obj.set_int_col.begin(); it != managed_obj.set_int_col.end(); ++it) {
            res.insert(*it);
        }
        CHECK(res == std::set<int64_t>({1, 2, 3}));
    }

    // All of the above but for managed objects

    SECTION("insert object") {
        auto barq = barq::native::db(std::move(config));

        AllTypesObjectLink link;
        link._id = 1;
        link.str_col = "foo";

        AllTypesObjectLink link2;
        link2._id = 2;
        link2.str_col = "bar";

        AllTypesObjectLink link3;
        link3._id = 3;
        link3.str_col = "bar";

        auto obj = barq::native::AllTypesObject();
        obj.set_obj_col.insert(&link);

        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        auto managed_link = barq.write([&]() {
            return barq.add(std::move(link3));
        });

        barq.write([&]() {
            managed_obj.set_obj_col.insert(&link2);
            managed_obj.set_obj_col.insert(managed_link);
        });
        CHECK(managed_obj.set_obj_col.size() == 3);
        barq.write([&]() {
            managed_obj.set_obj_col.insert(&link2);
            managed_obj.set_obj_col.insert(managed_link);
        });
        CHECK(managed_obj.set_obj_col.size() == 3);
    }

    SECTION("find") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        AllTypesObjectLink link;
        link._id = 1;
        link.str_col = "foo";

        AllTypesObjectLink link2;
        link2._id = 2;
        link2.str_col = "bar";

        AllTypesObjectLink link3;
        link3._id = 3;
        link3.str_col = "bar";

        auto managed_link_not_in_set = barq.write([&]() {
            return barq.add(std::move(link2));
        });

        auto managed_link = barq.write([&]() {
            barq.add(std::move(link2));
            return barq.add(std::move(link3));
        });

        barq.write([&]() {
            managed_obj.set_obj_col.insert(&link);
            managed_obj.set_obj_col.insert(&link);
            managed_obj.set_obj_col.insert(managed_link);
            managed_obj.set_obj_col.insert(managed_link);
        });
        CHECK(managed_obj.set_obj_col.size() == 2);

        auto it = managed_obj.set_obj_col.find(managed_link);
        CHECK(it != managed_obj.set_obj_col.end());
        CHECK((*it).str_col == "bar");
        CHECK(*it == barq.objects<AllTypesObjectLink>().where([](auto& o) { return o._id == 3; })[0]);
        auto it2 = managed_obj.set_obj_col.find(managed_link_not_in_set);
        CHECK(it2 == managed_obj.set_obj_col.end());
    }

    SECTION("erase") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        AllTypesObjectLink link;
        link._id = 1;
        link.str_col = "foo";

        AllTypesObjectLink link2;
        link2._id = 2;
        link2.str_col = "bar";

        auto managed_link = barq.write([&]() {
            return barq.add(std::move(link2));
        });

        barq.write([&]() {
            managed_obj.set_obj_col.insert(&link);
            managed_obj.set_obj_col.insert(managed_link);
        });

        auto it = managed_obj.set_obj_col.find(managed_link);
        barq.write([&]() {
            managed_obj.set_obj_col.erase(it);
        });
        it =  managed_obj.set_obj_col.find(managed_link);
        CHECK(it ==  managed_obj.set_obj_col.end());
        CHECK(managed_obj.set_obj_col.size() == 1);
        barq.write([&]() {
            managed_obj.set_obj_col.clear();
        });
        CHECK(managed_obj.set_obj_col.size() == 0);
    }

    SECTION("notifications") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        AllTypesObjectLink link;
        link._id = 1;
        link.str_col = "foo";

        AllTypesObjectLink link2;
        link2._id = 2;
        link2.str_col = "bar";

        auto managed_link = barq.write([&]() {
            return barq.add(std::move(link2));
        });

        size_t callback_count = 0;
        auto token = managed_obj.set_obj_col.observe([&](auto&& c) {
            callback_count++;
            if (callback_count == 2)
                CHECK(c.insertions.size() == 2);
            else if (callback_count == 3)
                CHECK(c.deletions.size() == 2);
        });
        barq.write([&]() {
            managed_obj.set_obj_col.insert(&link);
            managed_obj.set_obj_col.insert(managed_link);
        });
        barq.refresh();
        CHECK(callback_count == 2);

        barq.write([&]() {
            managed_obj.set_obj_col.clear();
        });
        barq.refresh();
        CHECK(callback_count == 3);
    }

    SECTION("detach()") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();

        AllTypesObjectLink link;
        link._id = 1;
        link.str_col = "foo";

        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        barq.write([&]() {
            managed_obj.set_obj_col.insert(&link);
        });

        auto other = managed_obj.set_obj_col.detach();
        CHECK(other.begin().operator*()->_id == link._id);
        CHECK(other.begin().operator*()->str_col == link.str_col);

        for (auto& o : other) {
            delete o->str_link_col;
            delete o;
        }

    }

    SECTION("set_algorithms") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::SetParentObject();

        AllTypesObjectLink link;
        link._id = 1;
        link.str_col = "foo";

        AllTypesObjectLink link2;
        link2._id = 2;
        link2.str_col = "bar";

        obj.set1.insert(&link);
        obj.set1.insert(&link2);

        obj.set2.insert(&link);

        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        barq.write([&]() {
            std::set_intersection(managed_obj.set1.begin(), managed_obj.set1.end(), managed_obj.set2.begin(), managed_obj.set2.end(), std::inserter(managed_obj.set3, managed_obj.set3.begin()));
        });
        CHECK(managed_obj.set3.size() == 1);
        CHECK(managed_obj.set3.begin().operator*()._id == 1);

        barq.write([&]() {
            managed_obj.set3.clear();
            std::set_union(managed_obj.set1.begin(), managed_obj.set1.end(), managed_obj.set2.begin(), managed_obj.set2.end(), std::inserter(managed_obj.set3, managed_obj.set3.begin()));
        });
        CHECK(managed_obj.set3.size() == 2);

        barq.write([&]() {
            managed_obj.set3.clear();
            std::set_difference(managed_obj.set1.begin(), managed_obj.set1.end(), managed_obj.set2.begin(), managed_obj.set2.end(), std::inserter(managed_obj.set3, managed_obj.set3.begin()));
        });
        CHECK(managed_obj.set3.size() == 1);

        barq.write([&]() {
            managed_obj.set3.clear();
            std::set_symmetric_difference(managed_obj.set1.begin(), managed_obj.set1.end(), managed_obj.set2.begin(), managed_obj.set2.end(), std::inserter(managed_obj.set3, managed_obj.set3.begin()));
        });
        CHECK(managed_obj.set3.size() == 1);
    }

    SECTION("iterator") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        auto managed_obj = barq.write([&]() {
            return barq.add(std::move(obj));
        });

        AllTypesObjectLink link;
        link._id = 1;
        link.str_col = "foo";

        AllTypesObjectLink link2;
        link2._id = 2;
        link2.str_col = "bar";

        barq.write([&]() {
            managed_obj.set_obj_col.insert(&link);
            managed_obj.set_obj_col.insert(&link2);
        });

        std::set<int64_t> res;
        for (const auto& x : managed_obj.set_obj_col) {
            res.insert(x._id);
        }
        CHECK(res == std::set<int64_t>({1, 2}));
        res.clear();

        for (auto it = managed_obj.set_obj_col.begin(); it != managed_obj.set_obj_col.end(); ++it) {
            res.insert(it.operator*()._id);
        }
        CHECK(res == std::set<int64_t>({1, 2}));
    }

    SECTION("object lifetime") {
        std::unique_ptr<managed<AllTypesObjectLink>> ptr;
        {
            auto barq = barq::native::db(std::move(config));
            auto obj = barq::native::AllTypesObject();
            auto managed_obj = barq.write([&]() {
                return barq.add(std::move(obj));
            });

            AllTypesObjectLink link;
            link._id = 1;
            link.str_col = "foo";

            AllTypesObjectLink link2;
            link2._id = 2;
            link2.str_col = "bar";

            AllTypesObjectLink link3;
            link3._id = 3;
            link3.str_col = "bar";

            auto managed_link_not_in_set = barq.write([&]() {
                return barq.add(std::move(link2));
            });

            auto managed_link = barq.write([&]() {
                barq.add(std::move(link2));
                return barq.add(std::move(link3));
            });

            barq.write([&]() {
                managed_obj.set_obj_col.insert(&link);
                managed_obj.set_obj_col.insert(&link);
                managed_obj.set_obj_col.insert(managed_link);
                managed_obj.set_obj_col.insert(managed_link);
            });
            CHECK(managed_obj.set_obj_col.size() == 2);

            auto it = managed_obj.set_obj_col.find(managed_link);
            ptr = std::make_unique<managed<AllTypesObjectLink>>(*it);
        }
        // Object should exist after scope exit
        CHECK(ptr->str_col == "bar");
    }
}

