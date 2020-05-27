#pragma once

#include <nodes/node.h>

class StringNode : public Node {
public:
    std::string text;
    std::vector<size_t> indices;

    StringNode(Parser &parser, Node *parent);
};
