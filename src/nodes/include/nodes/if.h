#pragma once

#include <nodes/node.h>

class IfNode : public Node {
public:
    bool hasValue();

    IfNode(Parser &parser, Node *parent);
};
