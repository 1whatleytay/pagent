#include <target/js/context.h>

#include <nodes/code.h>
#include <nodes/function.h>
#include <nodes/expression.h>

std::string JsContext::genFunction(FunctionNode *node) {
    Parameters params = node->parameters();

    std::vector<std::string> paramNames(params.size());
    for (size_t a = 0; a < params.size(); a++) {
        if (params[a].reference)
            paramNames[a] = reserveName(params[a].reference);
        else
            paramNames[a] = params[a].name;
    }

    Node *body = node->body();

    std::stringstream content;

    if (body->type == Node::Type::Expression) {
        content << fmt::format("return {}", genExpression(body->as<ExpressionNode>()));
    } else if (body->type == Node::Type::Code) {
        if (node->init) {
            node->searchHere([this, &content](Node *node) {
                if (node->type == Node::Type::Variable && node->as<VariableNode>()->init) {
                    std::string name = reserveName(node);

                    content << fmt::format("\nthis.{} = {}", name, name);
                }

                return false;
            });
        }

        content << genCode(body->as<CodeNode>());

        if (node->init)
            content << "\nreturn this";
    }

    return fmt::format("\n{}{}({}) {{ {}\n}}",
        node->isMethod() ? "" : "function ", reserveName(node), fmt::join(paramNames, ", "), indent(content.str()));
}
