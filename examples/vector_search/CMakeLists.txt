# The examples are built as part of the SDK, not on their own — they rely on the
# `barq_native` target and its include paths. Configuring this folder directly
# gives a confusing "sdk.hpp not found", so fail early with the right command.
if(NOT TARGET barq_native)
    message(FATAL_ERROR
        "Build the examples from the SDK root, not this folder:\n"
        "  cmake -G Ninja -S <barq-native> -B build -DBARQ_CORE_SOURCE_DIR=/path/to/barq-core\n"
        "  cmake --build build --target vector_search")
endif()

add_executable(vector_search main.cpp)
target_link_libraries(vector_search PRIVATE barq_native)

# Bake the absolute path to the sample data in, so the demo finds it no matter
# where it is run from.
target_compile_definitions(vector_search PRIVATE
        DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/deep1b_sample.json")
