#pragma once

#include <nodes/node.h>

class RootNode : public Node {
public:
    RootNode(Parser &parser, Node *parent);
};
