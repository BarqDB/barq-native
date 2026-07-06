#include <barq_native/managed_uuid.hpp>
#include <barq_native/rbool.hpp>

namespace barq::native {
    __barq_native_build_query(==, equal, barq::native::uuid)
    __barq_native_build_query(!=, not_equal, barq::native::uuid)
    __barq_native_build_optional_query(==, equal, barq::native::uuid)
    __barq_native_build_optional_query(!=, not_equal, barq::native::uuid)
}