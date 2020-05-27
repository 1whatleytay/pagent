#pragma once

#include <nodes/node.h>

class ForNode : public Node {
public:
    enum class Operator {
        While,
        In,
    };

    Operator op = Operator::While;

    bool hasValue();

    ForNode(Parser &parser, Node *parent);
};
