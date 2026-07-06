#include "../main.hpp"
#include "test_objects.hpp"

TEST_CASE("embedded_objects") {
    barq_path path;
    barq::native::db_config config;
    config.set_path(path);
    SECTION("observe") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::AllTypesObject();
        barq::native::AllTypesObjectEmbedded embedded;
        embedded.str_col = "foo";
        obj.opt_embedded_obj_col = &embedded;

        auto managed_obj = barq.write([&obj, &barq]() {
            return barq.add(std::move(obj));
        });
        CHECK(managed_obj.opt_embedded_obj_col->str_col == "foo");

        int run_count = 0;
        auto opt_embedded_obj_col = managed_obj.opt_embedded_obj_col;
        auto token = opt_embedded_obj_col->observe([&run_count](auto change) {
            run_count++;
            if (change.is_deleted) return;
            CHECK(change.object->str_col == "123");
        });
        barq.write([&managed_obj]() {
            managed_obj.str_col = "abc";
        });
        barq.refresh();
        barq.write([&opt_embedded_obj_col]() {
            opt_embedded_obj_col->str_col  = "123";
        });
        barq.refresh();
        CHECK(run_count == 1);
        barq.write([&managed_obj, &barq] {
            barq.remove(managed_obj);
        });
        barq.refresh();
        CHECK(run_count == 2);
    }

    SECTION("depth") {
        auto barq = barq::native::db(std::move(config));
        auto obj = barq::native::EmbeddedDepthObject();

        auto emb3 = barq::native::EmbeddedDepth3();
        emb3.str_col = "emb3";

        auto emb2 = barq::native::EmbeddedDepth2();
        emb2.str_col = "emb2";
        emb2.embedded_link = &emb3;

        auto emb1 = barq::native::EmbeddedDepth1();
        emb1.str_col = "emb1";
        emb1.embedded_link = &emb2;
        obj.embedded_link = &emb1;

        auto managed_obj = barq.write([&obj, &barq]() {
            return barq.add(std::move(obj));
        });

        CHECK(managed_obj.embedded_link->str_col == "emb1");
        CHECK(managed_obj.embedded_link->embedded_link->str_col == "emb2");
        CHECK(managed_obj.embedded_link->embedded_link->embedded_link->str_col == "emb3");

        auto r = barq.objects<barq::native::EmbeddedDepthObject>();
        auto o = r[0];
        CHECK(o.embedded_link->str_col == "emb1");
        CHECK(o.embedded_link->embedded_link->str_col == "emb2");
        CHECK(o.embedded_link->embedded_link->embedded_link->str_col == "emb3");
    }
}
