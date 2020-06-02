#pragma once

#include <nodes/method.h>

class JsRoot;

class JsFunction {
public:
    std::string name;
    std::vector<std::string> paramNames;

    bool isMethod = false;

    std::string content;

    std::string build();

    JsFunction(JsRoot &root, MethodNode *node);
};
