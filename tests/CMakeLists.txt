cmake_minimum_required(VERSION 3.16)
project(barq_native_tests)
set(CMAKE_CXX_STANDARD 17)

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.0.1 # or a later release
)
FetchContent_MakeAvailable(Catch2)

if (DEFINED VCPKG_TOOLCHAIN)
    set(BUILD_FROM_PACKAGE_MANAGER ON)
else()
    set(BUILD_FROM_PACKAGE_MANAGER OFF)
endif()

add_executable(barq_native_db_tests
                main.hpp
                main.cpp
                db/test_objects.hpp
                db/binary_tests.cpp
                db/date_tests.cpp
                db/decimal_tests.cpp
                db/embedded_object_tests.cpp
                db/list_tests.cpp
                db/map_tests.cpp
                db/mixed_tests.cpp
                db/object_id_tests.cpp
                db/object_tests.cpp
                db/optional_tests.cpp
                db/query_tests.cpp
                db/barq_tests.cpp
                db/results_tests.cpp
                db/scheduler_tests.cpp
                db/string_tests.cpp
                db/performance_tests.cpp
                db/numeric_tests.cpp
                db/set_tests.cpp
                db/frozen_tests.cpp
                db/uuid_tests.cpp)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor /bigobj")
endif()

if(DEFINED VCPKG_OVERLAY_PORTS)
    find_package(barq_native CONFIG REQUIRED)
    set(BARQ_NATIVE_TARGET barq_native)
else()
    set(BARQ_NATIVE_TARGET barq_native)
endif()

if(NOT APPLE AND NOT WINDOWS_STORE AND NOT ANDROID)
    if(DEFINED VCPKG_OVERLAY_PORTS)
        find_package(libuv CONFIG REQUIRED)
        target_link_libraries(barq_native_db_tests PUBLIC $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)
    endif()
    target_compile_definitions(barq_native_db_tests PUBLIC BARQ_HAVE_UV)
endif()

if (MSVC AND ENABLE_STATIC)
    set_property(TARGET barq_native_db_tests PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    set_property(TARGET Catch2 PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_link_libraries(barq_native_db_tests PUBLIC ${BARQ_NATIVE_TARGET} Catch2::Catch2)

add_test(barq_native_tests barq_native_db_tests)
