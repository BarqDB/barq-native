#include <barq_native/managed_decimal.hpp>
#include <barq_native/internal/bridge/decimal128.hpp>
#include <barq_native/rbool.hpp>

namespace barq::native {
    __barq_native_build_query(==, equal, barq::native::decimal128)
    __barq_native_build_query(!=, not_equal, barq::native::decimal128)
    __barq_native_build_query(>, greater, barq::native::decimal128)
    __barq_native_build_query(>=, greater_equal, barq::native::decimal128)
    __barq_native_build_query(<, less, barq::native::decimal128)
    __barq_native_build_query(<=, less_equal, barq::native::decimal128)
    __barq_native_build_optional_query(==, equal, barq::native::decimal128)
    __barq_native_build_optional_query(!=, not_equal, barq::native::decimal128)

#define managed_decimal_arithmetic(op) \
    barq::native::decimal128 managed<barq::native::decimal128>::operator op(const decimal128& o) { \
        auto old_val = m_obj->template get<internal::bridge::decimal128>(m_key); \
        auto new_val = old_val.operator op(o.m_decimal); \
        m_obj->template set<internal::bridge::decimal128>(this->m_key, new_val); \
        return new_val.operator ::barq::native::decimal128(); \
    } \

    managed_decimal_arithmetic(+)
    managed_decimal_arithmetic(-)
    managed_decimal_arithmetic(*)
    managed_decimal_arithmetic(/)

#define managed_decimal_arithmetic_assignment(op, arithmetic_op) \
    managed<barq::native::decimal128>& managed<barq::native::decimal128>::operator op(const decimal128& o) { \
        auto old_val = m_obj->template get<internal::bridge::decimal128>(m_key); \
        auto new_val = old_val.operator arithmetic_op(o.m_decimal); \
        m_obj->template set<internal::bridge::decimal128>(this->m_key, new_val); \
        return *this; \
    } \

    managed_decimal_arithmetic_assignment(+=, +)
    managed_decimal_arithmetic_assignment(-=, -)
    managed_decimal_arithmetic_assignment(*=, *)
    managed_decimal_arithmetic_assignment(/=, /)

    // MARK: Optional

#define managed_optional_decimal_arithmetic(op) \
    barq::native::decimal128 managed<std::optional<barq::native::decimal128>>::operator op(const decimal128& o) { \
        if (auto old_val = m_obj->template get_optional<internal::bridge::decimal128>(m_key)) { \
            auto new_val = old_val->operator op(o.m_decimal); \
            m_obj->template set<internal::bridge::decimal128>(this->m_key, new_val); \
            return new_val.operator ::barq::native::decimal128(); \
        } \
        throw std::runtime_error("Optional decimal128 property has null value"); \
    } \

    managed_optional_decimal_arithmetic(+)
    managed_optional_decimal_arithmetic(-)
    managed_optional_decimal_arithmetic(*)
    managed_optional_decimal_arithmetic(/)

#define managed_optional_decimal_arithmetic_assignment(op, arithmetic_op) \
    managed<std::optional<barq::native::decimal128>>&  managed<std::optional<barq::native::decimal128>>::operator op(const decimal128& o) { \
        if (auto old_val = m_obj->template get_optional<internal::bridge::decimal128>(m_key)) { \
        auto new_val = old_val->operator arithmetic_op(o.m_decimal); \
        m_obj->template set<internal::bridge::decimal128>(this->m_key, new_val); \
        return *this; \
        } \
        throw std::runtime_error("Optional decimal128 property has null value"); \
    } \

    managed_optional_decimal_arithmetic_assignment(+=, +)
    managed_optional_decimal_arithmetic_assignment(-=, -)
    managed_optional_decimal_arithmetic_assignment(*=, *)
    managed_optional_decimal_arithmetic_assignment(/=, /)
}
