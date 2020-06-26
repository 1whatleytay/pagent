#pragma once

#include <nodes/node.h>

class IfNode : public Node {
    void parseBody(Parser &parser);

public:
    bool hasValue();

    /*
     * TODO: verify that all the branches are actually expressions if hasValue
     * TODO: verify that all the branches are the same type or lower types if hasValue
     */

    IfNode(Parser &parser, Node *parent);
};
