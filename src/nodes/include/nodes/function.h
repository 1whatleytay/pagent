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

class MapResult {
public:
    bool matches = true;
    std::vector<ssize_t> map;
    std::string error;

    MapResult() = default;
    MapResult(std::string error);
};

class Parameters : public std::vector<Parameter> {
public:
    void add(Node *node);
    ssize_t find(const std::string &name);

    MapResult map(std::vector<Node *> values, std::map<std::string, size_t> names);
};

class FunctionNode : public Node {
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

    FunctionNode(Node *parent); // default init
    FunctionNode(Parser &parser, Node *parent, bool init, bool implicit);
};
