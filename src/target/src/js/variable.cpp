#include <target/js/context.h>

#include <nodes/expression.h>

std::string JsContext::genVariable(VariableNode *node) {
    // this is ok i bet
    Node *defaultValue = node->defaultValue();

    return fmt::format("\n{}{} = {}",
        (node->isField() && !node->shared) ? "" : "let ", reserveName(node),
        defaultValue ? genExpression(defaultValue->as<ExpressionNode>()) : "null");
}
