#pragma once

#include <target/error.h>
#include <target/js/type.h>
#include <target/js/action.h>
#include <target/js/function.h>

#include <nodes/root.h>
#include <nodes/variable.h>

#include <map>
#include <memory>
#include <string>

std::string indent(const std::string &content);

class JsRoot {
    size_t id;

    std::map<TypeNode *, std::shared_ptr<JsType>> types;
    std::map<VariableNode *, std::shared_ptr<JsAction>> variables;
    std::map<MethodNode *, std::shared_ptr<JsFunction>> functions;

public:
    Node *view = nullptr;
    Node *scene = nullptr;
    Node *text = nullptr;
    Node *button = nullptr;

    size_t nextId();

    std::string build();

    JsRoot(RootNode *node);
};
