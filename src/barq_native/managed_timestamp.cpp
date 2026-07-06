#include <barq_native/managed_timestamp.hpp>
#include <barq_native/rbool.hpp>

namespace barq::native {
    __barq_native_build_query(==, equal, std::chrono::time_point<std::chrono::system_clock>)
    __barq_native_build_query(!=, not_equal, std::chrono::time_point<std::chrono::system_clock>)
    __barq_native_build_query(>, greater, std::chrono::time_point<std::chrono::system_clock>)
    __barq_native_build_query(>=, greater_equal, std::chrono::time_point<std::chrono::system_clock>)
    __barq_native_build_query(<, less, std::chrono::time_point<std::chrono::system_clock>)
    __barq_native_build_query(<=, less_equal, std::chrono::time_point<std::chrono::system_clock>)

    __barq_native_build_optional_query(==, equal, std::chrono::time_point<std::chrono::system_clock>)
    __barq_native_build_optional_query(!=, not_equal, std::chrono::time_point<std::chrono::system_clock>)
}
