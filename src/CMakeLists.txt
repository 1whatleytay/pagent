add_subdirectory(nodes)
add_subdirectory(tests)
add_subdirectory(target)

add_executable(page main.cpp)
target_link_libraries(page PUBLIC nodes tests target fmt)

configure_file(standard/standard.page ${CMAKE_CURRENT_BINARY_DIR}/standard.page COPYONLY)
