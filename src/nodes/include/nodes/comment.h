#pragma once

#include <nodes/node.h>

class CommentNode : public Node {
public:
    std::string content;

    CommentNode(Parser &parser, Node *parent);
};
