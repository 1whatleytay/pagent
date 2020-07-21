#include <target/js/context.h>

#include <nodes/if.h>
#include <nodes/for.h>
#include <nodes/code.h>
#include <nodes/type.h>
#include <nodes/array.h>
#include <nodes/lambda.h>
#include <nodes/number.h>
#include <nodes/string.h>
#include <nodes/boolean.h>
#include <nodes/function.h>
#include <nodes/reference.h>
#include <nodes/expression.h>

std::string JsContext::genLiteral(Node *node) {
    switch (node->type) {
        case Node::Type::Number:
            return fmt::format("{}", node->as<NumberNode>()->value);
        case Node::Type::Boolean:
            return node->as<BooleanNode>()->value ? "true" : "false";
        case Node::Type::String: {
            StringNode *e = node->as<StringNode>();

            std::string output = e->text;

            size_t correction = 0;
            for (size_t a = 0; a < e->indices.size(); a++) {
                std::string insert = fmt::format("${{ {} }}", genExpression(e->children[a]->as<ExpressionNode>()));

                output.insert(e->indices[a] + correction, insert);
                correction += insert.size();
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
            if (selected->type == Node::Type::Function) {
                if (selected->as<FunctionNode>()->init) {
                    if (selected->parent == view || selected->parent == scene
                        || selected->parent == clickable || selected->parent == link) {
                        bool isScene = selected->parent == scene;

                        bool isRootView = e->parent->parent->type == Node::Type::Type;

                        std::string style;
                        std::string attributes;

                        if (e->hasCall) {
                            std::vector<Node *> passed = e->getParameters();
                            Parameters parameters = selected->as<FunctionNode>()->parameters();

                            MapResult map = parameters.map(passed, e->names);

                            if (!map.matches)
                                throw CompileError(selected, "Could not match constructor to text.");

                            style = genStyle(parameters, map.map, passed);
                            attributes = genAttributes(parameters, map.map, passed);
                        }

                        std::stringstream content;

                        if (e->hasContent) {
                            CodeNode *nodeContent = e->getContent()->as<CodeNode>();

                            for (const auto &child : nodeContent->children) {
                                if (child->type == Node::Type::Comment)
                                    continue;

                                if (child->type != Node::Type::Expression)
                                    throw CompileError(child.get(),
                                        "Cannot have non expression in content for view/scene.");

                                content << fmt::format("\n${{ $insert({}) }}", genExpression(child->as<ExpressionNode>()));
                            }
                        }

                        std::string tag = "div";

                        if (selected->parent == clickable) {
                            tag = "button";
                        }

                        if (selected->parent == link) {
                            tag = "a";
                        }

                        return fmt::format("`<{}{} style=\"display: {};{}\"{}>{}\n</{}>`",
                            tag, isRootView ? " id=\"${this.$uuid}\"" : "",
                            isScene ? "flex" : "block", style, attributes, indent(content.str()), tag);
                    } else if (selected->parent == text || selected->parent == button
                        || selected->parent == image || selected->parent == icon) {
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

                        std::string attributes = genAttributes(parameters, map.map, passed);

                        std::vector<std::string> result(parameters.size());

                        for (size_t a = 0; a < map.map.size(); a++) {
                            result[map.map[a]] = genExpression(passed[a]->as<ExpressionNode>());
                        }

                        if (selected->parent == text) {
                            return fmt::format("`<div{}{}>${{ $clean({}) }}</div>`",
                                styleTemplate, attributes, result[0]);
                        } else if (selected->parent == button) {
                            return fmt::format("`<button{}{}>${{ $clean({}) }}</button>`",
                                styleTemplate, attributes, result[0]);
                        } else if (selected->parent == image) {
                            return fmt::format("`<img{}{} src=\"${{ {} }}\" alt=\"Image\">`",
                                styleTemplate, attributes, result[0]);
                        } else if (selected->parent == icon) {
                            return fmt::format("`<svg><use xlink:href=\"${{ {} }}{}{}\"></use></svg>`",
                                result[0], styleTemplate, attributes);
                        }
                    }
                } else {
                    if (selected == print) {
                        std::vector<Node *> passed = e->getParameters();
                        Parameters parameters = selected->as<FunctionNode>()->parameters();

                        MapResult map = parameters.map(passed, e->names);

                        std::vector<std::string> result(parameters.size());

                        for (size_t a = 0; a < map.map.size(); a++) {
                            result[map.map[a]] = genExpression(passed[a]->as<ExpressionNode>());
                        }

                        return fmt::format("console.log({})", result[0]);
                    }
                }
            }

            return genReference(e);
        }
        case Node::Type::If: {
            IfNode *e = node->as<IfNode>();

            if (e->hasValue()) {
                return genTernary(e);
            } else {
                std::stringstream stream;

                bool first = true;
                size_t index = 0;
                while (index < e->children.size()) {
                    if (index + 1 < e->children.size()) {
                        if (!first) {
                            stream << " else ";
                        } else {
                            first = false;
                        }

                        stream << "if ";

                        stream << "(" << genExpression(e->children[index]->as<ExpressionNode>()) << ") ";

                        index++;
                    } else {
                        stream << " else ";
                    }

                    stream << "{" << indent(genCode(e->children[index]->as<CodeNode>())) << "\n}";
                    index++;
                }

                return stream.str();
            }
        }
        case Node::Type::For: {
            ForNode *e = node->as<ForNode>();

            if (e->op == ForNode::Operator::While)
                throw CompileError(e, "Unimplemented while for loop.");

            if (!e->hasValue())
                throw CompileError(e, "Unsupported no value for loop.");

            return fmt::format("{}.map(({}) => {})",
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
