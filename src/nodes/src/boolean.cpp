#include <nodes/boolean.h>

BooleanNode::BooleanNode(Parser &parser, Node *parent) : Node(parent, Type::Boolean) {
    std::string next = parser.next();

    if (next == "true")
        value = true;
    else if (next == "false")
        value = false;
    else
        throw ParseError(parser, "Internal boolean error, unknown boolean literal.");
}
