add_library(target STATIC
    include/target/error.h
    include/target/js/context.h

    src/js/context.cpp
    src/js/code.cpp
    src/js/type.cpp
    src/js/literal.cpp
    src/js/reference.cpp
    src/js/style.cpp
    src/js/attributes.cpp
    src/js/ternary.cpp
    src/js/enum.cpp
    src/js/variable.cpp
    src/js/expression.cpp
    src/js/function.cpp
    src/js/variable.cpp)

target_include_directories(target PUBLIC include)
target_link_libraries(target PUBLIC nodes tests)
