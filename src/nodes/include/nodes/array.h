#pragma once

#include <nodes/node.h>

class ArrayNode : public Node {
public:
    Typename evaluate(std::vector<Node *> visited = { });

    ArrayNode(Parser &parser, Node *parent);
};
