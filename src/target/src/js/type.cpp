#include <target/js/context.h>

#include <nodes/type.h>
#include <nodes/function.h>
#include <nodes/expression.h>

#include <fmt/format.h>

#include <sstream>

std::string JsContext::genType(TypeNode *node) {
    std::stringstream body;
    std::stringstream global;

    for (const auto &child : node->children) {
        switch (child->type) {
            case Node::Type::Variable:
                if (child->as<VariableNode>()->shared)
                    global << "\n" << genVariable(child->as<VariableNode>());
                else
                    body << "\n" << genVariable(child->as<VariableNode>());
                break;

            case Node::Type::Expression: // parsed as expression but is really UI, should be shown in template
                body << fmt::format("\n$template() {{\n{}\n}}",
                    indent(fmt::format("return {}", genExpression(child->as<ExpressionNode>()))));
                break;

            case Node::Type::Function:
                body << genFunction(child->as<FunctionNode>());
                break;

            case Node::Type::Route:
            case Node::Type::Comment:
                break; // OK

            default:
                assert(false);
        }
    }

    return fmt::format("\nclass {}{} {{{}\n}}{}",
        reserveName(node), node->isPage ? " extends $Component" : "", indent(body.str()), global.str());
}
