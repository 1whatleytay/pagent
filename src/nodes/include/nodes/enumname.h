#pragma once

#include <nodes/node.h>

class EnumnameNode : public Node {
public:
    std::string name;

    EnumnameNode(Parser &parser, Node *parent);
};
