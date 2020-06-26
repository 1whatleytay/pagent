#include <nodes/enum.h>

#include <nodes/enumname.h>

void EnumNode::verify() {
    searchHere([this](Node *node) {
        if (node == this)
            return false;

        if (name == getName(node))
            throw VerifyError("Duplicate name {} in scope.", name);

        return false;
    });

    Node::verify();
}

Typename EnumNode::evaluate() {
    return Typename(name);
}

EnumNode::EnumNode(Parser &parser, Node *parent) : Node(parent, Type::Enum) {
    if (parser.next() != "enum")
        throw ParseError(parser, "Internal error, expected enum.");

    name = parser.next();

    if (parser.next() != "{")
        throw ParseError(parser, "Expected {{ after enum name, but got {}.", parser.last());

    while (!parser.empty()) {
        if (parser.peek() == "}")
            break;

        children.push_back(std::make_shared<EnumnameNode>(parser, this));

        if (parser.peek() == ",")
            parser.next();
    }

    parser.next(); // }
}
