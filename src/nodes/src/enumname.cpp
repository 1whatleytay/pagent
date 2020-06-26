#include <nodes/enumname.h>

EnumnameNode::EnumnameNode(Parser &parser, Node *parent) : Node(parent, Node::Type::Enumname) {
    name = parser.next(); // YEP its that simple
    // i just need a node to pass around for references
}
