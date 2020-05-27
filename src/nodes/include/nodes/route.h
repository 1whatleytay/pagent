#pragma once

#include <nodes/node.h>

class RouteNode : public Node {
public:
    RouteNode(Parser &parser, Node *parent);
};
