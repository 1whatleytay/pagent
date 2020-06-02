#pragma once

#include <nodes/node.h>
#include <nodes/typename.h>

#include <functional>

// capture the node you need :|
using Evaluator = std::function<Typename()>;

class VariableNode : public Node {
public:
    std::string name;
    bool hasExplicitType = false;

    bool fake = false;
    Evaluator evaluator;

    bool init = false;

    void verify() override;

    bool isField();
    Node *defaultValue();

    Typename evaluate(std::vector<Node *> visited = { });

    VariableNode(std::string name, Node *parent);
    VariableNode(Parser &parser, Node *parent, bool init);
};
