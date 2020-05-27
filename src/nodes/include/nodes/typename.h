#pragma once

#include <nodes/node.h>

class TypenameNode : public Node {
public:
    Typename content;

    TypenameNode(Parser &parser, Node *parent);
};
