add_library(target STATIC
    include/target/error.h
    include/target/js/root.h
    include/target/js/type.h
    include/target/js/expression.h
    include/target/js/action.h
    include/target/js/function.h

    src/js/root.cpp
    src/js/type.cpp
    src/js/expression.cpp
    src/js/action.cpp
    src/js/function.cpp)

target_include_directories(target PUBLIC include)
target_link_libraries(target PUBLIC nodes tests)
