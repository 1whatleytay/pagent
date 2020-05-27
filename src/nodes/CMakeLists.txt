add_library(nodes STATIC
    include/nodes/error.h
    include/nodes/node.h
    include/nodes/attributes.h
    include/nodes/operator.h
    include/nodes/root.h
    include/nodes/enum.h
    include/nodes/type.h
    include/nodes/string.h
    include/nodes/typename.h
    include/nodes/variable.h
    include/nodes/expression.h
    include/nodes/code.h
    include/nodes/statement.h
    include/nodes/method.h
    include/nodes/route.h
    include/nodes/number.h
    include/nodes/lambda.h
    include/nodes/reference.h
    include/nodes/if.h
    include/nodes/for.h
    include/nodes/array.h

    src/node.cpp
    src/attributes.cpp
    src/root.cpp
    src/enum.cpp
    src/type.cpp
    src/string.cpp
    src/typename.cpp
    src/variable.cpp
    src/expression.cpp
    src/code.cpp
    src/statement.cpp
    src/method.cpp
    src/route.cpp
    src/number.cpp
    src/lambda.cpp
    src/reference.cpp
    src/if.cpp
    src/for.cpp
    src/array.cpp
    )

target_include_directories(nodes PUBLIC include)

target_link_libraries(nodes PUBLIC fmt langutil)
