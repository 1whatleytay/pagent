add_subdirectory(nodes)
add_subdirectory(tests)

add_executable(page main.cpp)
target_link_libraries(page PUBLIC nodes tests fmt)
