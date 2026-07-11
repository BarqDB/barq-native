#include <barq_native/managed_numeric.hpp>

namespace barq::native {
    __barq_native_build_query(==, equal, bool)
    __barq_native_build_query(!=, not_equal, bool)

    __barq_native_build_optional_query(==, equal, int64_t)
    __barq_native_build_optional_query(!=, not_equal, int64_t)

    __barq_native_build_optional_query(==, equal, double)
    __barq_native_build_optional_query(!=, not_equal, double)

    __barq_native_build_optional_query(==, equal, float)
    __barq_native_build_optional_query(!=, not_equal, float)

    __barq_native_build_optional_query(==, equal, bool)
    __barq_native_build_optional_query(!=, not_equal, bool)
}