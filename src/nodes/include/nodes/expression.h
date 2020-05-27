#pragma once

#include <nodes/node.h>

#include <nodes/typename.h>

#include <map>

class ExpressionNode : public Node {
public:
    enum class Operator {
        Literal,

        // Unary
        Not,
        Negative,
        Has,

        // Binary
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo,
        And,
        Or,
        Equals,
        NotEquals,
        GreaterThan,
        LesserThan,
        GreaterThanOrEqual,
        LesserThanOrEqual,
        Assign,
        AddAssign,
        SubtractAssign,
        MultiplyAssign,
        DivideAssign,
        ModuloAssign,
    };

    Operator op;

    Typename evaluate(std::vector<Node *> visited = { });

    void verify() override;

    static std::shared_ptr<ExpressionNode> parse(Parser &parser, Node *parent);

    // Only parses literals/var references.
    ExpressionNode(Operator op, Node *parent);
    ExpressionNode(Parser &parser, Node *parent);
};
