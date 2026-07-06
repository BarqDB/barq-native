#include <barq_native/internal/bridge/uuid.hpp>

#include <barq_native/types.hpp>
#include <barq/uuid.hpp>

namespace barq::native::internal::bridge {
    uuid::uuid() {
        m_uuid = UUID::UUIDBytes();
    }

    uuid::uuid(const std::string& v) {
        m_uuid = UUID(v).to_bytes();
    }

    uuid::uuid(const std::array<uint8_t, 16>& v) {
        m_uuid = v;
    }

    uuid::uuid(const ::barq::native::uuid &v)
    {
        m_uuid = UUID(v.to_string()).to_bytes();
    }

    uuid::uuid(const UUID &v) {
        m_uuid = UUID(v).to_bytes();
    }

    std::string uuid::to_string() const {
        return UUID(m_uuid).to_string();
    }

    uuid::operator ::barq::native::uuid() const {
        return ::barq::native::uuid(*this);
    }

    uuid::operator UUID() const {
        return UUID(m_uuid);
    }

    std::string uuid::to_base64() const {
        return UUID(m_uuid).to_base64();
    }

    std::array<uint8_t, 16> uuid::to_bytes() const {
        return m_uuid;
    }

#define __cpp_barq_gen_uuid_op(op) \
    bool operator op(const uuid& a, const uuid& b) { \
        return a.operator UUID() op b.operator UUID(); \
    }

    __cpp_barq_gen_uuid_op(==)
    __cpp_barq_gen_uuid_op(!=)
    __cpp_barq_gen_uuid_op(>)
    __cpp_barq_gen_uuid_op(<)
    __cpp_barq_gen_uuid_op(>=)
    __cpp_barq_gen_uuid_op(<=)
}