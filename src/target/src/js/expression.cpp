#include <target/js/expression.h>

#include <target/js/root.h>

#include <nodes/for.h>
#include <nodes/code.h>
#include <nodes/type.h>
#include <nodes/array.h>
#include <nodes/lambda.h>
#include <nodes/number.h>
#include <nodes/string.h>
#include <nodes/reference.h>
#include <nodes/statement.h>
#include <nodes/expression.h>

std::string jsReference(JsRoot &root, ReferenceNode *node, Node *scope) {
    Node *current = node->selectFrom(node->dereferenceThis(scope));

    std::string params;

    if (node->hasCall) {
        if (current->type != Node::Type::Method)
            throw CompileError(current, "Expected call on method.");

        std::vector<Node *> passed = node->getParameters();
        Parameters parameters = current->as<MethodNode>()->parameters();

        std::vector<ssize_t> output;

        bool success = parameters.map(passed, node->names, output);

        if (!success)
            throw CompileError(current, "Could not match constructor to text.");

        std::vector<std::string> result(parameters.parameters.size());

        for (size_t a = 0; a < output.size(); a++) {
            result[output[a]] = jsExpression(root, passed[a]->as<ExpressionNode>());
        }

        for (std::string &e : result) {
            if (e.empty()) {
                e = "null";
            }
        }

        params = fmt::format("({})", fmt::join(result, ", "));
    }

    if (node->hasContent)
        throw CompileError(node, "Unimplemented reference node content.");

    std::string first;

    if (!scope && current->parent->type == Node::Type::Type) {
        first = "this.";
    }

    first += node->content;

    std::string last;

    if (node->next())
        last = jsReference(root, node->next(), node->findType(current));

    return fmt::format("{}{}", first, params, last);
}

std::string jsLiteral(JsRoot &root, Node *node, bool isUI) {
    switch (node->type) {
        case Node::Type::Number:
            return fmt::format("{}", node->as<NumberNode>()->value);
        case Node::Type::String: {
            StringNode *e = node->as<StringNode>();

            std::string output = e->text;

            for (size_t a = 0; a < e->indices.size(); a++) {
                output.insert(e->indices[a], jsExpression(root, e->children[a]->as<ExpressionNode>(), false));
            }

            return fmt::format("\'{}\'", output);
        }
        case Node::Type::Array: {
            ArrayNode *e = node->as<ArrayNode>();
            std::vector<std::string> contents(e->children.size());

            for (size_t a = 0; a < e->children.size(); a++) {
                contents[a] = jsExpression(root, e->children[a]->as<ExpressionNode>(), isUI);
            }

            return fmt::format("[ {} ]", fmt::join(contents, ", "));
        }
        case Node::Type::Reference: {
            ReferenceNode *e = node->as<ReferenceNode>();

            Node *selected = e->select();

            // standard initializers, view, scene, type, node
            if (selected->type == Node::Type::Method && selected->as<MethodNode>()->init) {
                if (selected->parent == root.view || selected->parent == root.scene) {
                    bool isScene = selected->parent == root.scene;

                    std::stringstream content;

                    if (e->hasContent) {
                        CodeNode *nodeContent = e->getContent()->as<CodeNode>();

                        for (const auto &child : nodeContent->children) {
                            if (child->type != Node::Type::Expression)
                                throw CompileError(child.get(),
                                    "Cannot have non expression in content for view/scene.");

                            content << jsExpression(root, child->as<ExpressionNode>(), isUI);
                        }
                    }

                    return fmt::format("`<div{}>\n{}\n</div>`",
                        isScene ? " style=\"display: flex;\"" : "", indent(fmt::format("${{ {} }}", content.str())));
                } else if (selected->parent == root.text || selected->parent == root.button) {
                    std::vector<Node *> passed = e->getParameters();
                    Parameters parameters = selected->as<MethodNode>()->parameters();

                    std::vector<ssize_t> output;

                    bool success = parameters.map(passed, e->names, output);

                    if (!success)
                        throw CompileError(selected, "Could not match constructor to text.");

                    std::vector<std::string> result(parameters.parameters.size());

                    for (size_t a = 0; a < output.size(); a++) {
                        result[output[a]] = jsExpression(root, passed[a]->as<ExpressionNode>(), isUI);
                    }

                    if (selected->parent == root.text) {
                        return fmt::format("`<div>${{ {} }}</div>`", result[0]);
                    } else if (selected->parent == root.button) {
                        if (result[1].empty()) // click:
                            return fmt::format("`<button>${{ {} }}</button>`", result[0]);
                        else
                            return fmt::format( // urk
                                "`<button onclick=\"$callEvent(${{ $routeEvent({}) }})\">${{ {} }}</button>`",
                                result[0], result[1]);
                    }
                } else {
                    throw CompileError(selected, "Internal custom render type error.");
                }
            }

            return jsReference(root, e);
        }
        case Node::Type::For: {
            ForNode *e = node->as<ForNode>();

            if (e->op == ForNode::Operator::While)
                throw CompileError(e, "Unimplemented while for loop.");

            if (!e->hasValue())
                throw CompileError(e, "Unsupported no value for loop.");

            return fmt::format("{}.map(({}) => {}){}",
                jsExpression(root, e->children[1]->as<ExpressionNode>(), isUI),
                e->children[0]->as<VariableNode>()->name,
                jsExpression(root, e->children[2]->as<ExpressionNode>(), isUI),
                isUI ? ".join('')" : "");
        }
        case Node::Type::Lambda: {
            LambdaNode *e = node->as<LambdaNode>();

            std::vector<std::string> paramNames(e->paramCount);

            for (size_t a = 0; a < e->paramCount; a++) {
                paramNames[a] = e->children[a]->as<VariableNode>()->name;
            }

            Node *body = e->children[e->paramCount].get();

            std::string bodyText;

            if (body->type == Node::Type::Expression) {
                bodyText = jsExpression(root, body->as<ExpressionNode>());
            } else if (body->type == Node::Type::Code) {
                bodyText = fmt::format("{{ {} }}", jsBody(root, body->as<CodeNode>()));
            } else {
                assert(false);
            }

            return fmt::format("({}) => {}", fmt::join(paramNames, ", "), bodyText);
        }
        default:
            assert(false);
    }
}

std::string jsExpression(JsRoot &root, ExpressionNode *node, bool isUI) {
    switch (node->op) {
        case ExpressionNode::Operator::Literal:
            return jsLiteral(root, node->children[0].get());
        case ExpressionNode::Operator::Not:
            return fmt::format("(!{})", jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Negative:
            return fmt::format("(-{})", jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Has:
            return fmt::format("({} != null)", jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Add:
            return fmt::format("({} + {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Subtract:
            return fmt::format("({} - {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Multiply:
            return fmt::format("({} * {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Divide:
            return fmt::format("({} / {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Modulo:
            return fmt::format("({} % {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::And:
            return fmt::format("({} && {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Or:
            return fmt::format("({} || {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Equals:
            return fmt::format("({} === {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::NotEquals:
            return fmt::format("({} !== {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::GreaterThan:
            return fmt::format("({} > {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::LesserThan:
            return fmt::format("({} < {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::GreaterThanOrEqual:
            return fmt::format("({} >= {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::LesserThanOrEqual:
            return fmt::format("({} <= {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::Assign:
            return fmt::format("({} = {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::AddAssign:
            return fmt::format("({} += {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::SubtractAssign:
            return fmt::format("({} -= {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::MultiplyAssign:
            return fmt::format("({} *= {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::DivideAssign:
            return fmt::format("({} /= {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        case ExpressionNode::Operator::ModuloAssign:
            return fmt::format("({} %= {})",
                jsExpression(root, node->children[0]->as<ExpressionNode>(), isUI),
                jsExpression(root, node->children[1]->as<ExpressionNode>(), isUI));
        default:
            assert(false);
    }
}

std::string jsBody(JsRoot &root, CodeNode *node) {
    std::stringstream stream;

    for (const auto &child : node->children) {
        if (child->type == Node::Type::Expression) {
            stream << jsExpression(root, child->as<ExpressionNode>());
        } else if (child->type == Node::Type::Statement) {
            StatementNode *e = child->as<StatementNode>();

            switch (e->op) {
                case StatementNode::Operator::Break:
                    return "break";
                case StatementNode::Operator::Continue:
                    return "continue";
                case StatementNode::Operator::Return:
                    if (e->children.empty())
                        return "return";
                    else
                        return fmt::format("return {}", jsExpression(root, e->children[0]->as<ExpressionNode>()));
                default:
                    assert(false);
            }
        } else {
            assert(false);
        }
    }

    return stream.str();
}
