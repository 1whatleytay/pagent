#include <target/js/context.h>

#include <nodes/if.h>
#include <nodes/expression.h>

std::string JsContext::genTernary(IfNode *node, size_t index) {
    std::string other;

    if ((index + 3) >= node->children.size()) { // is this branch with terminating else?
        other = genExpression(node->children[index + 2]->as<ExpressionNode>());
    } else {
        other = genTernary(node, index + 2);
    }

    return fmt::format("({} ? {} : {})",
        genExpression(node->children[index]->as<ExpressionNode>()),
        genExpression(node->children[index + 1]->as<ExpressionNode>()),
        other);
}
