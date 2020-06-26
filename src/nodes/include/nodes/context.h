#pragma once

#include <nodes/node.h>

class RootNode : public Node {
    std::string path;
public:
    void add(const RootNode &node);

    RootNode(Parser &parser, Node *parent, const std::string &path);
};
