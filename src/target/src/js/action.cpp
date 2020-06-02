#include <target/js/action.h>

#include <target/js/expression.h>

#include <nodes/expression.h>

#include <fmt/format.h>

std::string JsAction::build() {
    std::string setupText = setup.str();
    std::string statementText = statement.str();

    return fmt::format("{}{}{}\n", setupText, setupText.empty() ? "" : "\n", statementText);
}

void JsAction::addSetup(const std::string &text) {
    setup << text;
}

void JsAction::addStatement(const std::string &text) {
    statement << text;
}

JsAction::JsAction(JsRoot &root, VariableNode *node) {
    // this is ok i bet
    ExpressionNode *value = node->defaultValue()->as<ExpressionNode>();

    statement << fmt::format("{}{}{}", node->isField() ? "" : "let ", node->name,
        fmt::format(" = {}", value ? jsExpression(root, value) : "null"));
}
