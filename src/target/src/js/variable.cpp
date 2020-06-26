#include <target/js/context.h>

#include <nodes/expression.h>

std::string JsContext::genVariable(VariableNode *node) {
    // this is ok i bet
    ExpressionNode *value = node->defaultValue()->as<ExpressionNode>();

    return fmt::format("{}{} = {}",
        (node->isField() && !node->shared) ? "" : "let ", reserveName(node), value ? genExpression(value) : "null");
}
