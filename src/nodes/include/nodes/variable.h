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

    bool init = false;
    bool fake = false;
    bool shared = false;
    Evaluator evaluator;


    void verify() override;

    bool isField();
    Node *defaultValue();

    Typename evaluate(std::vector<Node *> visited = { });

    VariableNode(std::string name, Node *parent);
    VariableNode(Parser &parser, Node *parent, bool init = false, bool shared = false);
};
