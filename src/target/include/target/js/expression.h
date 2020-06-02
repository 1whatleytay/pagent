#pragma once

#include <string>

class JsRoot;

class Node;
class CodeNode;
class ReferenceNode;
class ExpressionNode;

std::string jsReference(JsRoot &root, ReferenceNode *node, Node *scope = nullptr);
std::string jsLiteral(JsRoot &root, Node *node, bool isUI = false);
std::string jsExpression(JsRoot &root, ExpressionNode *node, bool isUI = false);
std::string jsBody(JsRoot &root, CodeNode *node);
