#include <barq_native/internal/bridge/utils.hpp>
#include <barq_native/internal/bridge/status.hpp>

#include <barq/status.hpp>

namespace barq::native::internal::bridge {
    bool error_category::test(type cat) {
        return reinterpret_cast<ErrorCategory*>(&m_error_category)->test(static_cast<::barq::ErrorCategory::Type>(cat));
    }
    error_category& error_category::set(type cat) {
        reinterpret_cast<ErrorCategory*>(&m_error_category)->set(static_cast<::barq::ErrorCategory::Type>(cat));
        return *this;
    }
    void error_category::reset(type cat) {
        reinterpret_cast<ErrorCategory*>(&m_error_category)->reset(static_cast<::barq::ErrorCategory::Type>(cat));
    }
    bool error_category::operator==(const error_category& other) const {
        return reinterpret_cast<const ErrorCategory*>(&m_error_category) == reinterpret_cast<const ErrorCategory*>(&other.m_error_category);
    }
    bool error_category::operator!=(const error_category& other) const {
        return reinterpret_cast<const ErrorCategory*>(&m_error_category) != reinterpret_cast<const ErrorCategory*>(&other.m_error_category);
    }
    int error_category::value() const {
        return reinterpret_cast<const ErrorCategory*>(&m_error_category)->value();
    }

    status status::ok() {
        return ::barq::Status::OK();
    }

    status::status(const ::barq::Status& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_status) Status(other);
#else
        m_status = std::make_shared<::barq::Status>(other);
#endif
    }
    status::status(::barq::Status&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_status) Status(std::move(other));
#else
        m_status = std::make_shared<::barq::Status>(std::move(other));
#endif
    }
    status::status(const status& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_status) Status(*reinterpret_cast<const Status*>(&other.m_status));
#else
        m_status = other.m_status;
#endif
    }
    status::status(status&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_status) Status(std::move(*reinterpret_cast<Status*>(&other.m_status)));
#else
        m_status = std::move(other.m_status);
#endif
    }
    status& status::operator=(const status& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        *reinterpret_cast<Status*>(&m_status) = *reinterpret_cast<const Status*>(&other.m_status);
#else
        m_status = other.m_status;
#endif
        return *this;
    }
    status& status::operator=(status&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        *reinterpret_cast<Status*>(&m_status) = std::move(*reinterpret_cast<Status*>(&other.m_status));
#else
        m_status = std::move(other.m_status);
#endif
        return *this;
    }

    status::~status() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<Status*>(&m_status)->~Status();
#endif
    }

    bool status::is_ok() const noexcept {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<const Status*>(&m_status)->is_ok();
#else
        return m_status->is_ok();
#endif
    }
    const std::string& status::reason() const noexcept {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<const Status*>(&m_status)->reason();
#else
        return m_status->reason();
#endif
    }

    std::string_view status::code_string() const noexcept {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<const Status*>(&m_status)->code_string();
#else
        return m_status->code_string();
#endif
    }

    status::operator ::barq::Status() const noexcept {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return *reinterpret_cast<const Status*>(&m_status);
#else
        return *m_status;
#endif
    }

}