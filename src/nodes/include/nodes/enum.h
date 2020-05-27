#pragma once

#include <nodes/node.h>

class EnumNode : public Node {
public:
    std::string name;
    std::vector<std::string> elements;

    void verify() override;

    EnumNode(Parser &parser, Node *parent);
};
