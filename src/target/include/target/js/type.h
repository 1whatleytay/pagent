#pragma once

#include <target/js/action.h>
#include <target/js/function.h>

#include <map>
#include <string>

class JsRoot;

class TypeNode;
class MethodNode;
class VariableNode;
class ExpressionNode;

class JsType {
    JsRoot &root;

public:
    std::string name;
    bool isPage;

    std::map<VariableNode *, std::shared_ptr<JsAction>> variables;
    std::map<MethodNode *, std::shared_ptr<JsFunction>> methods;

    JsAction templateBody;

    std::string build();

    JsType(JsRoot &root, TypeNode *node);
};
