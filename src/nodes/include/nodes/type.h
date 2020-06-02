#pragma once

#include <nodes/node.h>

class TypeNode : public Node {
public:
    std::string name;

    bool isPage = false;

    void verify() override;

    Typename evaluate();

    TypeNode(Parser &parser, Node *parent);
};
