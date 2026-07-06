#include <barq_native/managed_primary_key.hpp>

namespace barq::native {

#define __barq_native_build_pk_query(op, name, type, rhs_type) \
    rbool managed<primary_key<type>>::operator op(const rhs_type& rhs) const noexcept { \
        if (this->m_rbool_query) { \
            return this->m_rbool_query->name(m_key, rhs); \
        } \
        return serialize(detach().value) op serialize(rhs); \
    } \

// Int needs to be cast to int64_t
#define __barq_native_build_int_pk_query(op, name, type, rhs_type, cast) \
    rbool managed<primary_key<type>>::operator op(const rhs_type& rhs) const noexcept { \
        if (this->m_rbool_query) { \
            return this->m_rbool_query->name(m_key, serialize((cast)rhs)); \
        } \
        return serialize(detach().value) op serialize((cast)rhs); \
    }  \

    __barq_native_build_pk_query(==, equal, int64_t, int64_t)
    __barq_native_build_pk_query(!=, not_equal, int64_t, int64_t)
    __barq_native_build_pk_query(>, greater, int64_t, int64_t)
    __barq_native_build_pk_query(<, less, int64_t, int64_t)
    __barq_native_build_pk_query(>=, greater_equal, int64_t, int64_t)
    __barq_native_build_pk_query(<=, less_equal, int64_t, int64_t)

    __barq_native_build_int_pk_query(==, equal, int64_t, int, int64_t)
    __barq_native_build_int_pk_query(!=, not_equal, int64_t, int, int64_t)
    __barq_native_build_int_pk_query(>, greater, int64_t, int, int64_t)
    __barq_native_build_int_pk_query(<, less, int64_t, int, int64_t)
    __barq_native_build_int_pk_query(>=, greater_equal, int64_t, int, int64_t)
    __barq_native_build_int_pk_query(<=, less_equal, int64_t, int, int64_t)

    __barq_native_build_pk_query(==, equal, std::string, std::string)
    __barq_native_build_pk_query(!=, not_equal, std::string, std::string)

    __barq_native_build_pk_query(==, equal, barq::native::uuid, barq::native::uuid)
    __barq_native_build_pk_query(!=, not_equal, barq::native::uuid, barq::native::uuid)

    __barq_native_build_pk_query(==, equal, barq::native::object_id, barq::native::object_id)
    __barq_native_build_pk_query(!=, not_equal, barq::native::object_id, barq::native::object_id)

   rbool managed<primary_key<std::string>>::operator ==(const char* rhs) const noexcept {
        if (this->m_rbool_query) {
            return this->m_rbool_query->equal(m_key, std::string(rhs));
        }
        return serialize(detach().value) == serialize(std::string(rhs));
    }

    rbool managed<primary_key<std::string>>::operator !=(const char* rhs) const noexcept {
        if (this->m_rbool_query) {
            return this->m_rbool_query->not_equal(m_key, std::string(rhs));
        }
        return serialize(detach().value) != serialize(std::string(rhs));
    }

    rbool managed<primary_key<std::optional<std::string>>>::operator ==(const char* rhs) const noexcept {
        if (this->m_rbool_query) {
            return this->m_rbool_query->equal(m_key, std::string(rhs));
        }
        return serialize(detach().value) == serialize(std::string(rhs));
    }

    rbool managed<primary_key<std::optional<std::string>>>::operator !=(const char* rhs) const noexcept {
        if (this->m_rbool_query) {
            return this->m_rbool_query->not_equal(m_key, std::string(rhs));
        }
        return serialize(detach().value) != serialize(std::string(rhs));
    }

    // MARK: Optional

    __barq_native_build_pk_query(>, greater, std::optional<int64_t>, int64_t)
    __barq_native_build_pk_query(<, less, std::optional<int64_t>, int64_t)
    __barq_native_build_pk_query(>=, greater_equal, std::optional<int64_t>, int64_t)
    __barq_native_build_pk_query(<=, less_equal, std::optional<int64_t>, int64_t)

    __barq_native_build_int_pk_query(>, greater, std::optional<int64_t>, int, int64_t)
    __barq_native_build_int_pk_query(<, less, std::optional<int64_t>, int, int64_t)
    __barq_native_build_int_pk_query(>=, greater_equal, std::optional<int64_t>, int, int64_t)
    __barq_native_build_int_pk_query(<=, less_equal, std::optional<int64_t>, int, int64_t)

#define __barq_native_build_optional_pk_query(op, name, type, rhs_type) \
    rbool managed<primary_key<std::optional<type>>>::operator op(const rhs_type& rhs) const noexcept { \
        if (this->m_rbool_query) {                 \
            return this->m_rbool_query->name(m_key, rhs);  \
        } \
        return serialize(detach()) op serialize(rhs); \
    } \

    __barq_native_build_optional_pk_query(==, equal, int64_t, std::optional<int64_t>)
    __barq_native_build_optional_pk_query(!=, not_equal, int64_t, std::optional<int64_t>)


    __barq_native_build_optional_pk_query(==, equal, std::string, std::optional<std::string>)
    __barq_native_build_optional_pk_query(!=, not_equal, std::string, std::optional<std::string>)

    __barq_native_build_optional_pk_query(==, equal, barq::native::uuid, std::optional<barq::native::uuid>)
    __barq_native_build_optional_pk_query(!=, not_equal, barq::native::uuid, std::optional<barq::native::uuid>)

    __barq_native_build_optional_pk_query(==, equal, barq::native::object_id, std::optional<barq::native::object_id>)
    __barq_native_build_optional_pk_query(!=, not_equal, barq::native::object_id, std::optional<barq::native::object_id>)
}
