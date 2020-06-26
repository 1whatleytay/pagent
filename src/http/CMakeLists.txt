add_library(http STATIC
    include/http/http.h

    src/http.cpp)

target_include_directories(http PUBLIC include)
