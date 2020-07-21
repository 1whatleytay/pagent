#include <nodes/expression.h>

#include <nodes/if.h>
#include <nodes/for.h>
#include <nodes/code.h>
#include <nodes/array.h>
#include <nodes/number.h>
#include <nodes/string.h>
#include <nodes/lambda.h>
#include <nodes/boolean.h>
#include <nodes/operator.h>
#include <nodes/reference.h>

std::vector<OperatorRule<ExpressionNode::Operator>> unaryRules = {
    { ExpressionNode::Operator::Not, { { "!" } } },
    { ExpressionNode::Operator::Has, { { "has" } } },
    { ExpressionNode::Operator::Negative, { { "-" } } },
};

std::vector<OperatorRule<ExpressionNode::Operator>> binaryRules = {
    { ExpressionNode::Operator::Literal, { { "-", ">" } }, /*cancel:*/ true },
    { ExpressionNode::Operator::Literal, { { "/", "/" } }, /*cancel:*/ true },
    { ExpressionNode::Operator::Literal, { { "/", "*" } }, /*cancel:*/ true },
    { ExpressionNode::Operator::Assign, { { "=" } } },
    { ExpressionNode::Operator::AddAssign, { { "+", "=" } } },
    { ExpressionNode::Operator::SubtractAssign, { { "-", "=" } } },
    { ExpressionNode::Operator::MultiplyAssign, { { "*", "=" } } },
    { ExpressionNode::Operator::DivideAssign, { { "/", "=" } } },
    { ExpressionNode::Operator::ModuloAssign, { { "%", "=" } } },
    { ExpressionNode::Operator::Add, { { "+" } } },
    { ExpressionNode::Operator::Subtract, { { "-" } } },
    { ExpressionNode::Operator::Multiply, { { "*" } } },
    { ExpressionNode::Operator::Modulo, { { "%" } } },
    { ExpressionNode::Operator::Divide, { { "/" } } },
    { ExpressionNode::Operator::Equals, { { "=", "=" } } },
    { ExpressionNode::Operator::NotEquals, { { "!", "=" } } },
    { ExpressionNode::Operator::GreaterThanOrEqual, { { ">", "=" } } },
    { ExpressionNode::Operator::GreaterThan, { { ">" } } },
    { ExpressionNode::Operator::LesserThanOrEqual, { { "<", "=" } } },
    { ExpressionNode::Operator::LesserThan, { { "<" } } },
    { ExpressionNode::Operator::And, { { "&", "&" } } },
    { ExpressionNode::Operator::Or, { { "|", "|" } } },
};

std::vector<ExpressionNode::Operator> binaryRulesOrder = {
    ExpressionNode::Operator::Add,
    ExpressionNode::Operator::Subtract,
    ExpressionNode::Operator::Multiply,
    ExpressionNode::Operator::Modulo,
    ExpressionNode::Operator::Divide,
    ExpressionNode::Operator::Equals,
    ExpressionNode::Operator::NotEquals,
    ExpressionNode::Operator::GreaterThanOrEqual,
    ExpressionNode::Operator::GreaterThan,
    ExpressionNode::Operator::LesserThanOrEqual,
    ExpressionNode::Operator::LesserThan,
    ExpressionNode::Operator::And,
    ExpressionNode::Operator::Or,
    ExpressionNode::Operator::Assign,
    ExpressionNode::Operator::AddAssign,
    ExpressionNode::Operator::SubtractAssign,
    ExpressionNode::Operator::MultiplyAssign,
    ExpressionNode::Operator::DivideAssign,
    ExpressionNode::Operator::ModuloAssign,
};

std::shared_ptr<ExpressionNode> ExpressionNode::parse(Parser &parser, Node *parent) {
    return std::dynamic_pointer_cast<ExpressionNode>(
        parseOperator<ExpressionNode::Operator>(
            parser, parent, unaryRules, binaryRules, binaryRulesOrder,
            [](Parser &parser, Node *parent) { return std::make_shared<ExpressionNode>(parser, parent); },
            [](Operator type, Node *parent) { return std::make_shared<ExpressionNode>(type, parent); }
        )
    );
}

Typename ExpressionNode::evaluate(std::vector<Node *> visited) {
    if (std::find(visited.begin(), visited.end(), this) != visited.end())
        throw VerifyError("Circular evaluation detected.");
    visited.push_back(this);

    switch (op) {
        case Operator::Literal:
            switch (children[0]->type) {
                case Type::Number:
                    return Typename::number;
                case Type::Boolean:
                    return Typename::boolean;
                case Type::String:
                    return Typename::string;
                case Type::Lambda:
                    return children[0]->as<LambdaNode>()->evaluate();
                case Type::Reference:
                    return children[0]->as<ReferenceNode>()->evaluate(visited);
                case Type::Array:
                    return children[0]->as<ArrayNode>()->evaluate(visited);
                case Type::If:
                    // make if evaluate this
                    return children[0]->as<IfNode>()->children[1]->as<ExpressionNode>()->evaluate(visited);
                default:
                    return Typename::null;
            }

        case Operator::Negative:
        case Operator::Add:
        case Operator::Subtract:
        case Operator::Multiply:
        case Operator::Divide:
        case Operator::Modulo:
            return Typename::number;

        case Operator::Not:
        case Operator::Has:
        case Operator::And:
        case Operator::Or:
        case Operator::Equals:
        case Operator::NotEquals:
        case Operator::GreaterThan:
        case Operator::LesserThan:
        case Operator::GreaterThanOrEqual:
        case Operator::LesserThanOrEqual:
            return Typename::boolean;

        case Operator::Assign:
            return children[1]->as<ExpressionNode>()->evaluate();

        default:
            return Typename::null;
    }
}

void ExpressionNode::verify() {
    switch (op) {
        case Operator::Literal:
            break; // not bothering

        case Operator::Not: {
            Typename thisType = children[0]->as<ExpressionNode>()->evaluate();

            if (thisType != Typename::boolean)
                throw VerifyError("Expected boolean type for operator but got {}.", thisType.name);

            break;
        }

        case Operator::Negative: {
            Typename thisType = children[0]->as<ExpressionNode>()->evaluate();

            if (thisType != Typename::number)
                throw VerifyError("Expected number type for operator but got {}.", thisType.name);

            break;
        }

        case Operator::Has: {
            if (!children[0]->as<ExpressionNode>()->evaluate().optional)
                throw VerifyError("Has operator must be used on an optional type.");

            break;
        }

        case Operator::Add:
        case Operator::Subtract:
        case Operator::Multiply:
        case Operator::Divide:
        case Operator::Modulo:
        case Operator::GreaterThan:
        case Operator::LesserThan:
        case Operator::GreaterThanOrEqual:
        case Operator::LesserThanOrEqual: {
            Typename thisType = children[0]->as<ExpressionNode>()->evaluate();
            Typename thatType = children[1]->as<ExpressionNode>()->evaluate();

            if (thisType != Typename::number)
                throw VerifyError("Expected number type for operator but got {}.", thisType.name);

            if (thatType != Typename::number)
                throw VerifyError("Expected number type for operator but got {}.", thatType.name);

            break;
        }

        case Operator::And:
        case Operator::Or: {
            Typename thisType = children[0]->as<ExpressionNode>()->evaluate();
            Typename thatType = children[1]->as<ExpressionNode>()->evaluate();

            if (thisType != Typename::boolean)
                throw VerifyError("Expected number type for operator but got {}.", thisType.name);

            if (thatType != Typename::boolean)
                throw VerifyError("Expected number type for operator but got {}.", thatType.name);

            break;
        }

        case Operator::Equals:
        case Operator::NotEquals:
        case Operator::Assign: {
            Typename thisType = children[0]->as<ExpressionNode>()->evaluate();
            Typename thatType = children[1]->as<ExpressionNode>()->evaluate();

            if (thisType != thatType)
                throw VerifyError("Cannot assign type {} to type {}.", thisType.toString(), thatType.toString());

            break;
        }

        default:
            break;
    }

    Node::verify();
}

ExpressionNode::ExpressionNode(Operator op, Node *parent) : Node(parent, Type::Expression), op(op) { }

ExpressionNode::ExpressionNode(Parser &parser, Node *parent) : Node(parent, Type::Expression) {
    op = Operator::Literal;

    bool lambda = false;
    parser.select();

    // lambda, identified by `{` or `()` or `(* :`
    if (parser.peek() == "(") {
        parser.next(); // (

        std::string first = parser.next();

        // otherwise first is variable name
        if (first == ")" || parser.peek() == ":")
            lambda = true;
    } else if (parser.peek() == "{") {
        lambda = true;
    }

    parser.back();

    if (lambda) {
        children.push_back(std::make_shared<LambdaNode>(parser, this));
    } else if (parser.peek() == "[") {
        children.push_back(std::make_shared<ArrayNode>(parser, this));
    } else if (parser.peek() == "if") {
        children.push_back(std::make_shared<IfNode>(parser, this));
    } else if (parser.peek() == "for") {
        children.push_back(std::make_shared<ForNode>(parser, this));
    } else if (parser.peek() == "true" || parser.peek() == "false") {
        children.push_back(std::make_shared<BooleanNode>(parser, this));
    } else {
        std::string peek = parser.peek();

        if (peek.empty())
            throw ParseError(parser, "Expected literal but got end of file.");

        if (peek == "}")
            throw ParseError(parser, "Expected expression but got }}.");

        if (std::isdigit(peek[0]) || peek == "-") {
            children.push_back(std::make_shared<NumberNode>(parser, this));
        } else if (peek == "\"" || peek == "\'") {
            children.push_back(std::make_shared<StringNode>(parser, this));
        } else {
            children.push_back(std::make_shared<ReferenceNode>(parser, this));
        }
    }
}
