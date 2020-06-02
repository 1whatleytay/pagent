#pragma once

#include <nodes/variable.h>

#include <sstream>

class JsRoot;

class JsAction {
    std::stringstream setup;
    std::stringstream statement;
public:
    std::string build();

    void addSetup(const std::string &text);
    void addStatement(const std::string &text);

    JsAction() = default;
    JsAction(JsRoot &root, VariableNode *node); // declare variable
};
