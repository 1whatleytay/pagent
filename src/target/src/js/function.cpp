#include <target/js/function.h>

#include <target/js/root.h>
#include <target/js/expression.h>

#include <nodes/code.h>
#include <nodes/expression.h>

std::string JsFunction::build() {
    return fmt::format("{}{}({}) {{ {} }}\n",
        isMethod ? "" : "function ", name, fmt::join(paramNames, ", "), indent(content));
}

JsFunction::JsFunction(JsRoot &root, MethodNode *node) {
    name = node->name;

    if (node->init)
        name = "$build";
    isMethod = node->isMethod();

    Parameters params = node->parameters();

    paramNames.resize(params.parameters.size());
    for (size_t a = 0; a < params.parameters.size(); a++) {
        paramNames[a] = params.parameters[a].name;
    }

    Node *body = node->body();

    if (body->type == Node::Type::Expression) {
        content = fmt::format("return {}", jsExpression(root, body->as<ExpressionNode>()));
    } else if (body->type == Node::Type::Code) {
        content = jsBody(root, body->as<CodeNode>());
    }
}
