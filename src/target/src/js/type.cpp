#include <target/js/type.h>

#include <target/js/root.h>
#include <target/js/expression.h>

#include <nodes/type.h>
#include <nodes/expression.h>

#include <fmt/format.h>

std::string JsType::build() {
    std::stringstream body;

    for (const auto &variable : variables) {
        body << variable.second->build();
    }

    for (const auto &method : methods) {
        body << method.second->build();
    }

    if (isPage) {
        body <<  fmt::format("$template() {{\n{}\n}}", indent(templateBody.build()));
    }

    return fmt::format("class {}{} {{\n{}\n}}", name, isPage ? " extends $Component" : "", indent(body.str()));
}

JsType::JsType(JsRoot &root, TypeNode *node) : root(root) {
    name = node->name;
    isPage = node->isPage;

    for (const auto &child : node->children) {
        switch (child->type) {
            case Node::Type::Variable:
                variables[child->as<VariableNode>()] = std::make_shared<JsAction>(root, child->as<VariableNode>());
                break;
            case Node::Type::Route:
                break; // should be scanned by parent
            case Node::Type::Expression: // parsed as expression but is really UI, should be shown in template
                templateBody.addStatement(
                    fmt::format("return {}", jsExpression(root, child->as<ExpressionNode>(), true)));
                break;
            case Node::Type::Method:
                methods[child->as<MethodNode>()] = std::make_shared<JsFunction>(root, child->as<MethodNode>());
                break;
            default:
                assert(false);
        }
    }
}
