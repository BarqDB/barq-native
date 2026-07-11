cmake_minimum_required(VERSION 3.20)

project(barq_native)

if("$ENV{BARQ_CI}" OR BARQ_USE_CCACHE)
    include(cmake/ccache.cmake)
endif()

# Project-wide build flags
set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor /bigobj")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set(BARQ_BUILD_LIB_ONLY ON CACHE BOOL "" FORCE)
set(BARQ_NO_TESTS ON CACHE BOOL "" FORCE)
set(BARQ_ENABLE_SYNC ON CACHE BOOL "" FORCE)
set(BARQ_ENABLE_ENCRYPTION ON CACHE BOOL "" FORCE)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(BARQ_USE_SYSTEM_OPENSSL ON CACHE BOOL "" FORCE)
endif()

include(FetchContent)
set(BARQ_CORE_GIT_REPOSITORY "https://github.com/BarqDB/barq-core.git" CACHE STRING "Barq Core git repository")
# Pinned to the exact barq-core revision this SDK is built and tested against —
# the one carrying the vector (knn) search work and the stale vector-index
# accessor fix — so fetch-based builds are reproducible instead of tracking a
# moving `main`. Bump deliberately when barq-core advances. Local development can
# point at a working tree instead with -DBARQ_CORE_SOURCE_DIR=/path/to/barq-core.
set(BARQ_CORE_GIT_TAG "8e788c45207e7753789a7371d80121c6b613a5d9" CACHE STRING "Barq Core git ref")
set(BARQ_CORE_SOURCE_DIR "" CACHE PATH "Optional local Barq Core checkout")

if(BARQ_CORE_SOURCE_DIR)
    FetchContent_Declare(barq_core SOURCE_DIR "${BARQ_CORE_SOURCE_DIR}")
else()
    FetchContent_Declare(
        barq_core
        GIT_REPOSITORY ${BARQ_CORE_GIT_REPOSITORY}
        GIT_TAG ${BARQ_CORE_GIT_TAG}
    )
endif()
FetchContent_MakeAvailable(barq_core)

set(BARQ_NO_CONFIG)
set(BARQ_ENABLE_SYNC)
set(BARQ_NO_TESTS)
set(BARQ_BUILD_LIB_ONLY)
set(BARQ_INSTALL_LIBEXECDIR)
set(BARQ_ENABLE_ENCRYPTION 1)
option(BARQ_ENABLE_EXPERIMENTAL "Enable experimental Barq Native APIs" OFF)
if(BARQ_ENABLE_EXPERIMENTAL)
    add_compile_definitions(BARQ_ENABLE_EXPERIMENTAL)
endif()

set(CMAKE_DEBUG_POSTFIX "-dbg")

list(APPEND CMAKE_MODULE_PATH
        "${barq_native_SOURCE_DIR}/tools/cmake" # for GetVersion
        "${barq_core_SOURCE_DIR}/tools/cmake")
if(NOT DEFINED BARQ_NATIVE_VERSION)
    include(GetVersion)
    set(BARQ_NATIVE_VERSION ${CONFIG_BARQ_NATIVE_VERSION})
endif()

add_compile_definitions(BARQ_NO_CONFIG)
add_compile_definitions(BARQ_ENABLE_SYNC)
add_compile_definitions(BARQ_ENABLE_ENCRYPTION)
add_compile_definitions(BARQ_INSTALL_LIBEXECDIR)
add_compile_definitions(BARQ_BUILD_LIB_ONLY)

# on Apple platforms we use the built-in CFRunLoop
# everywhere else it's libuv, except UWP where it doesn't build
if(NOT APPLE AND NOT WINDOWS_STORE AND NOT ANDROID)
    set(BARQ_HAVE_UV 1)
    add_compile_definitions(BARQ_HAVE_UV=1)
endif()

configure_file(src/barq_native/util/config.h.in include/barq_native/util/config.h)

add_subdirectory(src)
target_include_directories(${PROJECT_NAME}
        PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>

        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

if(BARQ_HAVE_UV)
    if(VCPKG_TOOLCHAIN)
        find_package(libuv CONFIG REQUIRED)
        if(BUILD_SHARED_LIBS)
            set(libuv_target libuv::uv)
        else()
            set(libuv_target libuv::uv_a)
        endif()
    else()
        option(BARQ_FETCH_MISSING_DEPENDENCIES "Download missing dependencies with CMake's FetchContent where possible" ON)
        if(BARQ_FETCH_MISSING_DEPENDENCIES)
            find_package(LibUV)
        else()
            find_package(LibUV REQUIRED)
        endif()
        if(LibUV_FOUND)
            set(libuv_target LibUV::LibUV)
        elseif(BARQ_FETCH_MISSING_DEPENDENCIES)
            message(STATUS "LibUV not found, building from source with FetchContent")
            include(FetchContent)
            set(libUV_Git_TAG "v1.48.0")
            FetchContent_Declare(
                    libuv
                    GIT_REPOSITORY https://github.com/libuv/libuv.git
                    GIT_TAG ${libUV_Git_TAG}
            )
            FetchContent_Populate(libuv)
            add_subdirectory(${libuv_SOURCE_DIR} ${libuv_BINARY_DIR} EXCLUDE_FROM_ALL)
            set(libuv_target uv_a)
        endif()
    endif()

    target_link_libraries(barq_native PUBLIC ${libuv_target})
endif()

target_link_libraries(barq_native PUBLIC Barq::ObjectStore)
set(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT_PATH})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(barq_native PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(barq_native PRIVATE -Wno-deprecated-literal-operator)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(barq_native PRIVATE stdc++fs)
endif()

if(MSVC AND NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    if(NOT BUILD_SHARED_LIBS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set_property(TARGET barq_native PROPERTY
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set_property(TARGET uv_a PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

install(EXPORT "${PROJECT_NAME}Targets"
        FILE "${PROJECT_NAME}Targets.cmake"
        DESTINATION cmake
        )

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION "${BARQ_NATIVE_VERSION}"
        COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION cmake
        )
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION cmake
        )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/barq_native/util/config.h
        DESTINATION include/barq_native/util
        COMPONENT devel)

if (BUILD_SHARED_LIBS)
    set_target_properties(barq_native
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )

    get_target_property(_VisPreset barq_native CXX_VISIBILITY_PRESET)
    if (NOT MSVC AND _VisPreset STREQUAL "hidden")
        set_target_properties(barq_native
        PROPERTIES
            CXX_VISIBILITY_PRESET "default"
            VISIBILITY_INLINES_HIDDEN OFF
        )
        message(WARNING "Setting barq_native's visibility to default."
                        " Hidden visibility is not supported.")
    endif()
endif()

if (NOT BARQ_NATIVE_NO_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
