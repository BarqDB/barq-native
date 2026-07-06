#include <barq_native/internal/bridge/object_schema.hpp>
#include <barq_native/internal/bridge/property.hpp>

#include <barq/object-store/object_schema.hpp>
#include <barq/object-store/property.hpp>

namespace barq::native::internal::bridge {
    object_schema::object_schema() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_schema) ObjectSchema();
#else
        m_schema = std::make_shared<ObjectSchema>();
#endif
    }

    object_schema::object_schema(const object_schema& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_schema) ObjectSchema(*reinterpret_cast<const ObjectSchema*>(&other.m_schema));
#else
        m_schema = other.m_schema;
#endif
    }

    object_schema& object_schema::operator=(const object_schema& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<ObjectSchema*>(&m_schema) = *reinterpret_cast<const ObjectSchema*>(&other.m_schema);
        }
#else
        m_schema = other.m_schema;
#endif
        return *this;
    }

    object_schema::object_schema(object_schema&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_schema) ObjectSchema(std::move(*reinterpret_cast<ObjectSchema*>(&other.m_schema)));
#else
        m_schema = std::move(other.m_schema);
#endif
    }

    object_schema& object_schema::operator=(object_schema&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<ObjectSchema*>(&m_schema) = std::move(*reinterpret_cast<ObjectSchema*>(&other.m_schema));
        }
#else
        m_schema = std::move(other.m_schema);
#endif
        return *this;
    }

    object_schema::~object_schema() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<ObjectSchema*>(&m_schema)->~ObjectSchema();
#endif
    }

    object_schema::object_schema(const barq::ObjectSchema &v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_schema) ObjectSchema(v);
#else
        m_schema = std::make_shared<ObjectSchema>(v);
#endif
    }


    ObjectSchema*  object_schema::get_object_schema() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<ObjectSchema*>(&m_schema);
#else
        return m_schema.get();
#endif
    }

    const ObjectSchema*  object_schema::get_object_schema() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<const ObjectSchema*>(&m_schema);
#else
        return m_schema.get();
#endif
    }


    object_schema::object_schema(const std::string &name, const std::vector<property> &properties,
                                 const std::string &primary_key,
                                 barq::native::internal::bridge::object_schema::object_type type) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_schema) ObjectSchema();
#else
        m_schema = std::make_shared<ObjectSchema>();
#endif

        get_object_schema()->name = name;
        std::transform(properties.begin(),
                       properties.end(),
                       get_object_schema()->persisted_properties.begin(),
                       [](const property& p) {
            return static_cast<Property>(p);
        });
        get_object_schema()->primary_key = primary_key;
        get_object_schema()->table_type = static_cast<ObjectSchema::ObjectType>(type);
    }
    uint32_t object_schema::table_key() {
        return get_object_schema()->table_key.value;
    }

    void object_schema::set_object_type(barq::native::internal::bridge::object_schema::object_type o) {
        get_object_schema()->table_type = static_cast<ObjectSchema::ObjectType>(o);
    }

    void object_schema::add_property(const barq::native::internal::bridge::property &v) {
        get_object_schema()->persisted_properties.push_back(v);
    }

    void object_schema::set_name(const std::string &name) {
        get_object_schema()->name = name;
    }
    std::string object_schema::get_name() const {
        return get_object_schema()->name;
    }
    property object_schema::property_for_name(const std::string &v) {
        return *get_object_schema()->property_for_name(v);
    }
    void object_schema::set_primary_key(const std::string &primary_key) {
        get_object_schema()->primary_key = primary_key;
    }
    object_schema::operator ObjectSchema() const {
        return *get_object_schema();
    }

    bool object_schema::operator==(const object_schema& rhs) {
        return get_object_schema()->name == rhs.get_object_schema()->name;
    }
}
