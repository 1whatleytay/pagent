#include <nodes/enum.h>

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

EnumNode::EnumNode(Parser &parser, Node *parent) : Node(parent, Type::Enum) {
    if (parser.next() != "enum")
        throw ParseError(parser, "Internal error, expected enum.");

    name = parser.next();

    if (parser.next() != "{")
        throw ParseError(parser, "Expected {{ after enum name, but got {}.", parser.last());

    while (!parser.empty()) {
        std::string next = parser.next();

        if (next == "}")
            break;

        elements.push_back(next);

        if (parser.peek() == ",")
            parser.next();
    }
}
