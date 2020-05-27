#pragma once

#include <nodes/node.h>

#include <map>

class ReferenceNode : public Node {
public:
    bool hasCall = false;
    bool hasContent = false;
    size_t paramCount = 0;

    std::string content;
    std::map<std::string, size_t> names;

    std::vector<Node *> dereference(Node *reference = nullptr);
    Node *select();
    Node *selectFrom(const std::vector<Node *> &nodes);
    Typename evaluate(std::vector<Node *> visited = { });

    std::shared_ptr<Node> steal(); // content

    ReferenceNode(Parser &parser, Node *parent);
};
