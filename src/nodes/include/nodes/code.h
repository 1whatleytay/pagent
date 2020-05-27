#pragma once

#include <nodes/node.h>

class CodeNode : public Node {
public:
    CodeNode(Node *parent);
    CodeNode(Parser &parser, Node *parent);
};
