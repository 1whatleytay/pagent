#pragma once

#include <nodes/node.h>

#include <map>

class TypeNode;

class ReferenceNode : public Node {
public:
    bool hasCall = false;
    bool hasContent = false;
    size_t paramCount = 0;

    std::string content;
    std::map<std::string, size_t> names;

    ReferenceNode *next();
    TypeNode *findType(Node *referenced);

    std::vector<Node *> dereference(Node *reference = nullptr);
    std::vector<Node *> dereferenceThis(Node *reference = nullptr);
    Node *select();
    Node *selectFrom(const std::vector<Node *> &nodes);
    Typename evaluate(std::vector<Node *> visited = { });

    Node *getContent();
    std::vector<Node *> getParameters();
    std::shared_ptr<Node> steal(); // content

    void verify() override;

    ReferenceNode(Parser &parser, Node *parent);
};
