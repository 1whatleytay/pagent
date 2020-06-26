#include <target/js/context.h>

#include <nodes/if.h>
#include <nodes/code.h>
#include <nodes/type.h>
#include <nodes/function.h>
#include <nodes/reference.h>
#include <nodes/expression.h>

std::string JsContext::genExpression(ExpressionNode *node) {
    switch (node->op) {
        case ExpressionNode::Operator::Literal:
            return genLiteral(node->children[0].get());
        case ExpressionNode::Operator::Not:
            return fmt::format("(!{})", genExpression(node->children[0]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Negative:
            return fmt::format("(-{})", genExpression(node->children[0]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Has:
            return fmt::format("({} != null)", genExpression(node->children[0]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Add:
            return fmt::format("({} + {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Subtract:
            return fmt::format("({} - {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Multiply:
            return fmt::format("({} * {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Divide:
            return fmt::format("({} / {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Modulo:
            return fmt::format("({} % {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::And:
            return fmt::format("({} && {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Or:
            return fmt::format("({} || {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Equals:
            return fmt::format("({} === {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::NotEquals:
            return fmt::format("({} !== {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::GreaterThan:
            return fmt::format("({} > {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::LesserThan:
            return fmt::format("({} < {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::GreaterThanOrEqual:
            return fmt::format("({} >= {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::LesserThanOrEqual:
            return fmt::format("({} <= {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::Assign:
            return fmt::format("({} = {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::AddAssign:
            return fmt::format("({} += {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::SubtractAssign:
            return fmt::format("({} -= {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::MultiplyAssign:
            return fmt::format("({} *= {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::DivideAssign:
            return fmt::format("({} /= {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        case ExpressionNode::Operator::ModuloAssign:
            return fmt::format("({} %= {})",
                genExpression(node->children[0]->as<ExpressionNode>()),
                genExpression(node->children[1]->as<ExpressionNode>()));
        default:
            assert(false);
    }
}
