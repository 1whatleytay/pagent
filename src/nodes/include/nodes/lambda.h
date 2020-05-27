#pragma once

#include <nodes/node.h>

class LambdaNode : public Node {
public:
    size_t paramCount = 0;

    Typename evaluate();

    LambdaNode(Parser &parser, Node *parent);
};
