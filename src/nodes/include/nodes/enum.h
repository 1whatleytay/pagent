#pragma once

#include <nodes/node.h>

class EnumNode : public Node {
public:
    std::string name;

    void verify() override;

    Typename evaluate();

    EnumNode(Parser &parser, Node *parent);
};
