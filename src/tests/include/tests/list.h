#pragma once

#include <nodes/node.h>

#include <string>
#include <vector>

class NodeList {
public:
    std::string line;

    std::vector<NodeList> children;

    static std::string getLine(Node *node);

    std::string toString(size_t indentation = 0) const;

    explicit NodeList(Node *node);
};
