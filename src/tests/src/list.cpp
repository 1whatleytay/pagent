#include <tests/list.h>

#include <nodes/if.h>
#include <nodes/for.h>
#include <nodes/enum.h>
#include <nodes/type.h>
#include <nodes/array.h>
#include <nodes/lambda.h>
#include <nodes/method.h>
#include <nodes/number.h>
#include <nodes/string.h>
#include <nodes/typename.h>
#include <nodes/variable.h>
#include <nodes/reference.h>
#include <nodes/statement.h>
#include <nodes/expression.h>

#include <fmt/format.h>

#include <sstream>

static std::string getExpressionOpName(ExpressionNode::Operator op) {
    switch (op) {
        case ExpressionNode::Operator::Literal: return "Literal";
        case ExpressionNode::Operator::Not: return "Not";
        case ExpressionNode::Operator::Negative: return "Negative";
        case ExpressionNode::Operator::Has: return "Has";
        case ExpressionNode::Operator::Add: return "Add";
        case ExpressionNode::Operator::Subtract: return "Subtract";
        case ExpressionNode::Operator::Multiply: return "Multiply";
        case ExpressionNode::Operator::Divide: return "Divide";
        case ExpressionNode::Operator::Modulo: return "Modulo";
        case ExpressionNode::Operator::And: return "And";
        case ExpressionNode::Operator::Or: return "Or";
        case ExpressionNode::Operator::Equals: return "Equals";
        case ExpressionNode::Operator::NotEquals: return "NotEquals";
        case ExpressionNode::Operator::GreaterThan: return "GreaterThan";
        case ExpressionNode::Operator::LesserThan: return "LesserThan";
        case ExpressionNode::Operator::GreaterThanOrEqual: return "GreaterThanOrEqual";
        case ExpressionNode::Operator::LesserThanOrEqual: return "LesserThanOrEqual";
        case ExpressionNode::Operator::Assign: return "Assign";
        case ExpressionNode::Operator::AddAssign: return "AddAssign";
        case ExpressionNode::Operator::SubtractAssign: return "SubtractAssign";
        case ExpressionNode::Operator::MultiplyAssign: return "MultiplyAssign";
        case ExpressionNode::Operator::DivideAssign: return "DivideAssign";
        case ExpressionNode::Operator::ModuloAssign: return "ModuloAssign";
        default: return "Unknown";
    }
}

static std::string getStatementOpName(StatementNode::Operator op) {
    switch (op) {
        case StatementNode::Operator::Return: return "Return";
        case StatementNode::Operator::Break: return "Break";
        case StatementNode::Operator::Continue: return "Continue";
        default: return "Unknown";
    }
}

std::string NodeList::getLine(Node *node) {
    switch (node->type) {
        case Node::Type::Root:
            return "Root";
        case Node::Type::Code:
            return "Code";
        case Node::Type::Enum: {
            EnumNode *e = node->as<EnumNode>();
            return fmt::format("Enum \"{}\" {{ {} }}", e->name, fmt::join(e->elements, ", "));
        }
        case Node::Type::Type: {
            TypeNode *e = node->as<TypeNode>();
            return fmt::format("Type \"{}\"", e->name);
        }
        case Node::Type::Variable: {
            VariableNode *e = node->as<VariableNode>();
            return fmt::format("Variable \"{}\": {}{}{}", e->name, e->evaluate().toString(),
                e->init ? " + init" : "", e->fake ? " + fake" : "");
        }
        case Node::Type::Expression: {
            ExpressionNode *e = node->as<ExpressionNode>();
            return fmt::format("Expression {}", getExpressionOpName(e->op));
        }
        case Node::Type::If: {
            IfNode *e = node->as<IfNode>();
            return fmt::format("If{}", e->hasValue() ? " + value" : "");
        }
        case Node::Type::For: {
            ForNode *e = node->as<ForNode>();
            return fmt::format("For{}", e->hasValue() ? " + value" : "");
        }
        case Node::Type::String: {
            StringNode *e = node->as<StringNode>();
            return fmt::format("String \"{}\" {{ {} }}", e->text, fmt::join(e->indices, ", "));
        }
        case Node::Type::Typename: {
            TypenameNode *e = node->as<TypenameNode>();
            return fmt::format("Typename \"{}\"", e->content.toString());
        }
        case Node::Type::Method: {
            MethodNode *e = node->as<MethodNode>();
            return fmt::format("Method{} {{ #{} }}{}",
                e->init ? "" : fmt::format(" \"{}\"", e->name), e->paramCount, e->init ? " + init" : "");
        }
        case Node::Type::Statement: {
            StatementNode *e = node->as<StatementNode>();
            return fmt::format("Statement {}", getStatementOpName(e->op));
        }
        case Node::Type::Page: {
            TypeNode *e = node->as<TypeNode>();
            return fmt::format("Page \"{}\"", e->name);
        }
        case Node::Type::Route:
            return "Route";
        case Node::Type::Number: {
            NumberNode *e = node->as<NumberNode>();
            return fmt::format("Number {}", e->value);
        }
        case Node::Type::Lambda: {
            LambdaNode *e = node->as<LambdaNode>();
            return fmt::format("Lambda {{ #{} }}", e->paramCount);
        }
        case Node::Type::Reference: {
            ReferenceNode *e = node->as<ReferenceNode>();
            std::stringstream names;
            for (const auto &name : e->names) {
                names << ", " << name.first << ": " << name.second;
            }
            return fmt::format("Reference \"{}\"{}{}", e->content,
                e->hasCall ? fmt::format(" + call {{ #{}{} }}", e->paramCount, names.str()) : "",
                e->hasContent ? " + content" : "");
        }
        case Node::Type::Array: {
            ArrayNode *e = node->as<ArrayNode>();
            return fmt::format("Array \"{}\"", e->evaluate().toString());
        }
        default:
            return "Unknown";
    }
}

std::string NodeList::toString(size_t indentation) const {
    std::stringstream stream;

    stream << std::string(indentation * 2, ' ') << line << "\n";

    for (const NodeList &child : children) {
        stream << child.toString(indentation + 1);
    }

    return stream.str();
}

NodeList::NodeList(Node *node) {
    line = getLine(node);

    children.reserve(node->children.size());

    for (size_t a = 0; a < node->children.size(); a++) {
        children.push_back(NodeList(node->children[a].get()));
    }
}
