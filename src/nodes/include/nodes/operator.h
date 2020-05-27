#pragma once

#include <string>
#include <vector>
#include <memory>

#include <nodes/node.h>

#include <langutil/parser.h>

template <typename T>
class OperatorRule {
public:
    T op;

    std::vector<std::vector<std::string>> syntax;

    bool cancel = false;

    bool matches(Parser &parser) const {
        parser.select();

        for (const std::vector<std::string> &syn : syntax) {
            bool match = true;
            for (const std::string &text : syn) {
                if (parser.next() != text) {
                    match = false;
                    break;
                }
            }

            if (cancel || !match)
                parser.back();

            if (match)
                return true;
        }

        return false;
    }
};

template <typename O>
std::shared_ptr<Node> parseOperator(Parser &parser, Node *parent,
    const std::vector<OperatorRule<O>> &unaryRules,
    const std::vector<OperatorRule<O>> &binaryRules,
    const std::vector<O> &bianryRulesOrder,
    const std::function<std::shared_ptr<Node>(Parser &parser, Node *parent)> &parseNode,
    const std::function<std::shared_ptr<Node>(O op, Node *parent)> &createNode) {
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<O> keywords;

    while (!parser.empty()) {
        bool match;

        // check for unary
        match = false;
        std::shared_ptr<Node> expression;
        std::shared_ptr<Node> lowestNode;
        do {
            for (const auto &rule : unaryRules) {
                match = rule.matches(parser);

                if (match) {
                    std::shared_ptr<Node> unaryNode = createNode(rule.op, parent);

                    if (!expression) {
                        expression = unaryNode;
                        lowestNode = unaryNode;
                    } else {
                        unaryNode->children.push_back(expression);
                        expression->parent = unaryNode.get();

                        expression = unaryNode;
                    }

                    break;
                }
            }
        } while(match);

        std::shared_ptr<Node> firstNode = parseNode(parser, parent);
        if (expression) {
            firstNode->parent = lowestNode.get();
            lowestNode->children.push_back(firstNode);
        } else {
            expression = firstNode;
        }
        nodes.push_back(expression);

        // check for binary
        match = false;
        for (const auto &rule : binaryRules) {
            match = rule.matches(parser);

            if (match) {
                if (rule.cancel) {
                    match = false;
                    break;
                }

                keywords.push_back(rule.op);
                break;
            }
        }

        if (!match)
            break;
    }

    for (const auto &op : bianryRulesOrder) {
        for (uint32_t a = 0; a < keywords.size(); a++) {
            if (op != keywords[a])
                continue;

            std::shared_ptr<Node> paramA = nodes[a];
            std::shared_ptr<Node> paramB = nodes[a + 1];

            std::shared_ptr<Node> expression = createNode(keywords[a], parent);

            paramA->parent = expression.get();
            paramB->parent = expression.get();

            expression->children.push_back(paramA);
            expression->children.push_back(paramB);

            keywords.erase(keywords.begin() + a);
            nodes.erase(nodes.begin() + a + 1);
            nodes[a] = expression;
            a--; // don't move cursor since we erased an element
        }
    }

    assert(nodes.size() == 1);
    assert(keywords.empty());

    return nodes[0];
}
