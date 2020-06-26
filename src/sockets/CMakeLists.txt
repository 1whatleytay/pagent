add_library(sockets STATIC
    include/sockets/sockets.h

    src/sockets.cpp
    )

target_include_directories(sockets PUBLIC include)
