#include <target/js/context.h>

#include <nodes/if.h>
#include <nodes/for.h>
#include <nodes/code.h>
#include <nodes/type.h>
#include <nodes/array.h>
#include <nodes/lambda.h>
#include <nodes/number.h>
#include <nodes/string.h>
#include <nodes/function.h>
#include <nodes/reference.h>
#include <nodes/expression.h>

std::string JsContext::genLiteral(Node *node) {
    switch (node->type) {
        case Node::Type::Number:
            return fmt::format("{}", node->as<NumberNode>()->value);
        case Node::Type::String: {
            StringNode *e = node->as<StringNode>();

            std::string output = e->text;

            for (size_t a = 0; a < e->indices.size(); a++) {
                output.insert(e->indices[a],
                    fmt::format("${{ {} }}", genExpression(e->children[a]->as<ExpressionNode>())));
            }

            return fmt::format("`{}`", output);
        }
        case Node::Type::Array: {
            ArrayNode *e = node->as<ArrayNode>();
            std::vector<std::string> contents(e->children.size());

            for (size_t a = 0; a < e->children.size(); a++) {
                contents[a] = genExpression(e->children[a]->as<ExpressionNode>());
            }

            return fmt::format("[ {} ]", fmt::join(contents, ", "));
        }
        case Node::Type::Reference: {
            ReferenceNode *e = node->as<ReferenceNode>();

            Node *selected = e->select();

            // standard initializers, view, scene, type, node
            if (selected->type == Node::Type::Function && selected->as<FunctionNode>()->init) {
                if (selected->parent == view || selected->parent == scene) {
                    bool isScene = selected->parent == scene;

                    std::string style;

                    if (e->hasCall) {
                        std::vector<Node *> passed = e->getParameters();
                        Parameters parameters = selected->as<FunctionNode>()->parameters();

                        MapResult map = parameters.map(passed, e->names);

                        if (!map.matches)
                            throw CompileError(selected, "Could not match constructor to text.");

                        style = genStyle(parameters, map.map, passed);
                    }

                    std::stringstream content;

                    if (e->hasContent) {
                        CodeNode *nodeContent = e->getContent()->as<CodeNode>();

                        for (const auto &child : nodeContent->children) {
                            if (child->type != Node::Type::Expression)
                                throw CompileError(child.get(),
                                                   "Cannot have non expression in content for view/scene.");

                            content << fmt::format("\n${{ $insert({}) }}", genExpression(child->as<ExpressionNode>()));
                        }
                    }

                    return fmt::format("`<div style=\"display: {};{}\">{}\n</div>`",
                                       isScene ? "flex" : "block", style, indent(content.str()));
                } else if (selected->parent == text || selected->parent == button) {
                    std::vector<Node *> passed = e->getParameters();
                    Parameters parameters = selected->as<FunctionNode>()->parameters();

                    MapResult map = parameters.map(passed, e->names);

                    if (!map.matches)
                        throw CompileError(selected, "Could not match constructor to text.");

                    std::string style = genStyle(parameters, map.map, passed);
                    std::string styleTemplate;
                    if (!style.empty()) {
                        styleTemplate = fmt::format(" style=\"{}\"", style);
                    }

                    std::vector<std::string> result(parameters.size());

                    for (size_t a = 0; a < map.map.size(); a++) {
                        result[map.map[a]] = genExpression(passed[a]->as<ExpressionNode>());
                    }

                    if (selected->parent == text) {
                        return fmt::format("`<div{}>${{ $clean({}) }}</div>`", styleTemplate, result[0]);
                    } else if (selected->parent == button) {
                        std::string onclickFormat;
                        if (!result[1].empty()) {
                            onclickFormat = fmt::format(
                                " onclick=\"$callEvent(${{ $routeEvent(() => {{ return ({})() }}) }})\"",
                                result[1]);
                        }

                        return fmt::format("`<button{}{}>${{ $clean({}) }}</button>`",
                            styleTemplate, onclickFormat, result[0]);
                    }
                } else {
                    // just pass to genReference
//                    throw CompileError(selected, "Internal custom render type error.");
                }
            }

            return genReference(e);
        }
        case Node::Type::If: {
            IfNode *e = node->as<IfNode>();

            if (e->hasValue()) {
                return genTernary(e);
            } else {
                assert(false);
            }
        }
        case Node::Type::For: {
            ForNode *e = node->as<ForNode>();

            if (e->op == ForNode::Operator::While)
                throw CompileError(e, "Unimplemented while for loop.");

            if (!e->hasValue())
                throw CompileError(e, "Unsupported no value for loop.");

            return fmt::format("{}.map(({}) => {}){}",
                genExpression(e->children[1]->as<ExpressionNode>()),
                e->children[0]->as<VariableNode>()->name,
                genExpression(e->children[2]->as<ExpressionNode>()));
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
                bodyText = genExpression(body->as<ExpressionNode>());
            } else if (body->type == Node::Type::Code) {
                bodyText = fmt::format("{{ {} }}", genCode(body->as<CodeNode>()));
            } else {
                assert(false);
            }

            return fmt::format("({}) => {}", fmt::join(paramNames, ", "), bodyText);
        }
        default:
            assert(false);
    }
}
