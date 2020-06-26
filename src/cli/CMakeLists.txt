add_library(cli STATIC
    include/cli/cli.h
    include/cli/watcher.h

    src/cli.cpp
    src/watcher.cpp)

target_include_directories(cli PUBLIC include)
target_link_libraries(cli PUBLIC http sockets target nodes)
