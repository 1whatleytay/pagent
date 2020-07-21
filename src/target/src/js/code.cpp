#include <target/js/context.h>

#include <nodes/code.h>
#include <nodes/type.h>
#include <nodes/function.h>
#include <nodes/statement.h>
#include <nodes/expression.h>

std::string JsContext::genCode(CodeNode *node) {
    std::stringstream stream;

    for (const auto &child : node->children) {
        switch (child->type) {
            case Node::Type::Expression: {
                std::string expression = genExpression(child->as<ExpressionNode>());
                // lazy, will just trim out ( ) from expression, sure to cause issues in the future
                if (!expression.empty() && child->as<ExpressionNode>()->op != ExpressionNode::Operator::Literal)
                    expression = expression.substr(1, expression.size() - 2);
                stream << "\n" << expression;
                break;
            }
            case Node::Type::Statement: {
                StatementNode *e = child->as<StatementNode>();

                switch (e->op) {
                    case StatementNode::Operator::Break:
                        stream << "\nbreak";
                        break;
                    case StatementNode::Operator::Continue:
                        stream << "\ncontinue";
                        break;
                    case StatementNode::Operator::Return:
                        if (e->children.empty()) {
                            stream << "\nreturn";
                        } else {
                            stream << fmt::format("\nreturn {}{}",
                                                  genExpression(e->children[0]->as<ExpressionNode>()));
                        }
                        break;
                    default:
                        assert(false);
                }
                break;
            }
            case Node::Type::Variable: {
                VariableNode *e = child->as<VariableNode>();

                stream << "\n" << genVariable(e);

                break;
            }
            default:
                throw CompileError(child.get(), "Unexpected node type in CodeNode.");
        }
    }

    Node *page = node->searchParents([](Node *node) {
        return node->type == Node::Type::Type && node->as<TypeNode>()->isPage;
    });

    // just check if its part of a ui component for now
    if (page && (node->parent->type != Node::Type::Function || !node->parent->as<FunctionNode>()->init)) {
        stream << "\nthis.$reload()";
    }

    return stream.str();
}
