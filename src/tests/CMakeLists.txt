add_library(tests STATIC
    include/tests/list.h

    src/list.cpp)

target_link_libraries(tests PUBLIC nodes)
target_include_directories(tests PUBLIC include)
