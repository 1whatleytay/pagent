#pragma once

#include <nodes/node.h>

class BooleanNode : public Node {
public:
    bool value = false;

    BooleanNode(Parser &parser, Node *parent);
};
