#pragma once

#include <nodes/node.h>

#include <map>
#include <vector>

class Parameter {
public:
    Typename type;
    std::string name;
    Node *reference = nullptr;
};

class Parameters {
public:
    std::vector<Parameter> parameters;

    void add(Node *node);
    ssize_t find(const std::string &name);

    bool map(std::vector<Node *> values, std::map<std::string, size_t> names, std::vector<ssize_t> &result);
};

class MethodNode : public Node {
public:
    std::string name;

    size_t paramCount = 0;
    bool hasReturnType = false;

    bool init = false;
    bool implicit = false;

    bool isMethod(); // oh dear the naming
    Node *body();

    Parameters parameters();

    void verify() override;
    Typename evaluate();
    Typename evaluateReturn();

    MethodNode(Node *parent); // default init
    MethodNode(Parser &parser, Node *parent, bool init, bool implicit);
};
