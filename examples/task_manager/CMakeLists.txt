# The examples are built as part of the SDK, not on their own — they rely on the
# `barq_native` target and its include paths. Configuring this folder directly
# gives a confusing "sdk.hpp not found", so fail early with the right command.
if(NOT TARGET barq_native)
    message(FATAL_ERROR
        "Build the examples from the SDK root, not this folder:\n"
        "  cmake -G Ninja -S <barq-native> -B build -DBARQ_CORE_SOURCE_DIR=/path/to/barq-core\n"
        "  cmake --build build --target task_manager")
endif()

add_executable(task_manager main.cpp)
target_link_libraries(task_manager PRIVATE barq_native)
