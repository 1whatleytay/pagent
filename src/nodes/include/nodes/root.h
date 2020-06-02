#pragma once

#include <nodes/node.h>

class RootNode : public Node {
    std::string path;
public:
    void add(const RootNode &node);

    static RootNode fromFile(const std::string &path);

    RootNode(Parser &parser, Node *parent, const std::string &path);
};
