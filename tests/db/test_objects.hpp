#ifndef BARQ_NATIVE_TEST_OBJECTS_HPP
#define BARQ_NATIVE_TEST_OBJECTS_HPP

#include "barq_native/sdk.hpp"

namespace barq::native {
    struct Dog;
    struct Person {
        primary_key<int64_t> _id;
        std::string name;
        int64_t age = 0;
        Dog* dog;
    };
    BARQ_SCHEMA(Person, _id, name, age, dog)
    struct Dog {
        primary_key<int64_t> _id;
        std::string name;
        std::string name2;
        std::string name3;
        int64_t foo2 = 0;
        std::string name4;

        int64_t age = 0;
        linking_objects<&Person::dog> owners;
    };
    BARQ_SCHEMA(Dog,
                 _id,
                 name,
                 name2,
                 name3,
                 foo2,
                 name4,
                 age,
                 owners)

    struct EmbeddedDepth3 {
        std::string str_col;
    };
    BARQ_EMBEDDED_SCHEMA(EmbeddedDepth3, str_col)

    struct EmbeddedDepth2 {
        std::string str_col;
        EmbeddedDepth3* embedded_link;
    };
    BARQ_EMBEDDED_SCHEMA(EmbeddedDepth2, str_col, embedded_link)

    struct EmbeddedDepth1 {
        std::string str_col;
        EmbeddedDepth2* embedded_link;
    };
    BARQ_EMBEDDED_SCHEMA(EmbeddedDepth1, str_col, embedded_link)

    struct EmbeddedDepthObject {
        EmbeddedDepth1* embedded_link;
    };
    BARQ_SCHEMA(EmbeddedDepthObject, embedded_link)

    struct AllTypesObjectEmbedded {
        int64_t _id;
        std::string str_col;
    };
    BARQ_EMBEDDED_SCHEMA(AllTypesObjectEmbedded, _id, str_col)

    struct StringObject {
        primary_key<int64_t> _id;
        std::string str_col;
    };
    BARQ_SCHEMA(StringObject, _id, str_col)

    struct AllTypesObjectLink {
        primary_key<int64_t> _id;
        std::string str_col;
        StringObject* str_link_col = nullptr;

        std::vector<std::string> list_str_col;
        std::vector<StringObject*> list_obj_col;
        std::set<std::string> set_str_col;
        std::set<StringObject*> set_obj_col;
        std::map<std::string, std::string> map_str_col;
        std::map<std::string, StringObject*> map_link_col;

    };
    BARQ_SCHEMA(AllTypesObjectLink, _id, str_col, str_link_col, list_str_col, list_obj_col, set_str_col, set_obj_col, map_str_col, map_link_col)

    struct SetParentObject {
        primary_key<int64_t> _id;
        std::set<AllTypesObjectLink *> set1;
        std::set<AllTypesObjectLink *> set2;
        std::set<AllTypesObjectLink *> set3;
    };
    BARQ_SCHEMA(SetParentObject, _id, set1, set2, set3)

    struct AllTypesObject {
        enum class Enum {
            one, two
        };

        primary_key<int64_t> _id;
        int64_t int_col;
        double double_col;
        bool bool_col;
        std::string str_col;
        std::string str_col2;

        Enum enum_col = Enum::one;
        std::chrono::time_point<std::chrono::system_clock> date_col;
        barq::native::uuid uuid_col;
        barq::native::object_id object_id_col;
        barq::native::decimal128 decimal_col;
        std::vector<std::uint8_t> binary_col;
        barq::native::mixed mixed_col;
        barq::native::mixed my_mixed_col;

        std::optional<int64_t> opt_int_col;
        std::optional<double> opt_double_col;
        std::optional<std::string> opt_str_col;
        std::optional<bool> opt_bool_col;
        std::optional<Enum> opt_enum_col;
        std::optional<std::chrono::time_point<std::chrono::system_clock>> opt_date_col;
        std::optional<barq::native::uuid> opt_uuid_col;
        std::optional<barq::native::object_id> opt_object_id_col;
        std::optional<barq::native::decimal128> opt_decimal_col;
        std::optional<std::vector<uint8_t>> opt_binary_col;
        AllTypesObjectLink* opt_obj_col = nullptr;
        AllTypesObjectEmbedded* opt_embedded_obj_col = nullptr;

        std::vector<int64_t> list_int_col;
        std::vector<double> list_double_col;
        std::vector<bool> list_bool_col;
        std::vector<std::string> list_str_col;
        std::vector<barq::native::uuid> list_uuid_col;
        std::vector<barq::native::object_id> list_object_id_col;
        std::vector<barq::native::decimal128> list_decimal_col;
        std::vector<std::vector<std::uint8_t>> list_binary_col;
        std::vector<std::chrono::time_point<std::chrono::system_clock>> list_date_col;
        std::vector<barq::native::mixed> list_mixed_col;
        std::vector<Enum> list_enum_col;
        std::vector<AllTypesObjectLink*> list_obj_col;
        std::vector<AllTypesObjectEmbedded*> list_embedded_obj_col;

        std::set<int64_t> set_int_col;
        std::set<double> set_double_col;
        std::set<bool> set_bool_col;
        std::set<std::string> set_str_col;
        std::set<barq::native::uuid> set_uuid_col;
        std::set<barq::native::object_id> set_object_id_col;
        std::set<std::vector<std::uint8_t>> set_binary_col;
        std::set<std::chrono::time_point<std::chrono::system_clock>> set_date_col;
        std::set<barq::native::mixed> set_mixed_col;
        std::set<AllTypesObjectLink*> set_obj_col;

        std::map<std::string, int64_t> map_int_col;
        std::map<std::string, double> map_double_col;
        std::map<std::string, bool> map_bool_col;
        std::map<std::string, std::string> map_str_col;
        std::map<std::string, barq::native::uuid> map_uuid_col;
        std::map<std::string, barq::native::object_id> map_object_id_col;
        std::map<std::string, barq::native::decimal128> map_decimal_col;
        std::map<std::string, std::vector<std::uint8_t>> map_binary_col;
        std::map<std::string, std::chrono::time_point<std::chrono::system_clock>> map_date_col;
        std::map<std::string, Enum> map_enum_col;
        std::map<std::string, barq::native::mixed> map_mixed_col;
        std::map<std::string, AllTypesObjectLink*> map_link_col;
        std::map<std::string, AllTypesObjectEmbedded*> map_embedded_col;
    };

    BARQ_SCHEMA(AllTypesObject,
                 _id, int_col, double_col, bool_col, str_col, enum_col, date_col, uuid_col, object_id_col, decimal_col, binary_col, mixed_col, my_mixed_col,
                 opt_int_col, opt_double_col, opt_str_col, opt_bool_col, opt_enum_col,
                 opt_date_col, opt_uuid_col, opt_object_id_col, opt_decimal_col, opt_binary_col, opt_obj_col, opt_embedded_obj_col,
                 list_int_col, list_double_col, list_bool_col, list_str_col, list_uuid_col, list_object_id_col, list_decimal_col, list_binary_col,
                 list_date_col, list_mixed_col, list_enum_col, list_obj_col, list_embedded_obj_col,
                 set_int_col, set_double_col, set_bool_col, set_str_col, set_uuid_col, set_object_id_col, set_binary_col,
                 set_date_col, set_mixed_col, set_obj_col,
                 map_int_col, map_double_col, map_bool_col, map_str_col, map_uuid_col, map_object_id_col, map_decimal_col, map_binary_col,
                 map_date_col, map_enum_col, map_mixed_col, map_link_col, map_embedded_col)
}


#endif//BARQ_NATIVE_TEST_OBJECTS_HPP
