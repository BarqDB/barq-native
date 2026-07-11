add_executable(vector_search main.cpp)
target_link_libraries(vector_search PRIVATE barq_native)

# Bake the absolute path to the sample data in, so the demo finds it no matter
# where it is run from.
target_compile_definitions(vector_search PRIVATE
        DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/deep1b_sample.json")
