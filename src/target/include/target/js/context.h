#pragma once

#include <target/error.h>

#include <nodes/context.h>
#include <nodes/variable.h>

#include <unordered_map>
#include <memory>
#include <string>
#include <sstream>

class Parameters;

class IfNode;
class CodeNode;
class EnumNode;
class TypeNode;
class FunctionNode;
class VariableNode;
class ReferenceNode;
class ExpressionNode;

class JsContext {
    RootNode *root;
    size_t id;

    std::unordered_map<Node *, std::string> names;

    std::string reserveName(Node *node);

    std::string indent(const std::string &content);

    std::string genLiteral(Node *node);
    std::string genCode(CodeNode *node);
    std::string genEnum(EnumNode *node);
    std::string genType(TypeNode *node);
    std::string genFunction(FunctionNode *node);
    std::string genVariable(VariableNode *node);
    std::string genReference(ReferenceNode *node, Node *scope = nullptr);
    std::string genExpression(ExpressionNode *node);
    std::string genTernary(IfNode *node, size_t index = 0);

    std::string genStyle(
        const Parameters &params, const std::vector<ssize_t> &output, const std::vector<Node *> &values);
public:
    Node *view = nullptr;
    Node *scene = nullptr;
    Node *text = nullptr;
    Node *button = nullptr;

    Node *textStyleTextSize = nullptr;
    Node *textStyleTextColor = nullptr;
    Node *textStyleTextAlign = nullptr;
    Node *buttonStyleTextSize = nullptr;
    Node *buttonStyleTextColor = nullptr;
    Node *buttonStyleTextAlign = nullptr;

    size_t nextId();

    std::string build();

    JsContext(RootNode *root);
};
