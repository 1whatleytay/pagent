#pragma once

#include <nodes/node.h>

class MethodNode;

class StatementNode : public Node {
public:
    enum class Operator {
        Return,
        Break,
        Continue
    };

    Operator op;

    void verify() override;

    StatementNode(Parser &parser, Node *parent);
};
