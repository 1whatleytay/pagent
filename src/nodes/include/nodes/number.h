#pragma once

#include <nodes/node.h>

class NumberNode : public Node {
public:
    double value;

    NumberNode(Parser &parser, Node *parent);
};
