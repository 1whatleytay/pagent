#include <nodes/type.h>

#include <nodes/route.h>
#include <nodes/method.h>
#include <nodes/variable.h>
#include <nodes/attributes.h>
#include <nodes/expression.h>

void TypeNode::verify() {
    Typename thisType(name);
    if (thisType == Typename::string || thisType == Typename::number || thisType == Typename::boolean)
        throw VerifyError("You cannot declare a type with builtin name {}.", thisType.toString());

    searchHere([this](Node *node) {
        if (node == this)
            return false;

        if (name == getName(node))
            throw VerifyError("Duplicate name {} in scope.", name);

        return false;
    });

    Node::verify();
}

Typename TypeNode::evaluate() {
    return Typename(name);
}

TypeNode::TypeNode(Parser &parser, Node *parent) : Node(parent, Type::Type) {
    std::string typeWord = parser.next();

    if (typeWord == "type")
        type = Type::Type;
    else if (typeWord == "page")
        type = Type::Page;
    else
        throw ParseError(parser, "Internal error, expected type.");

    name = parser.next();

    if (parser.next() != "{")
        throw ParseError(parser, "Expected {{ after type name, but got {}.", parser.last());

    while (!parser.empty()) { // init optional name: x
        if (parser.peek() == "}")
            break;

        auto attributes = parseAttributes(parser, { "init", "implicit", "native" });

        std::string next = parser.next();
        std::string action = parser.peek();
        parser.rollback();

        if (next == "var") {
            if (attributes["implicit"])
                throw ParseError(parser, "Attribute implicit used on variable declaration.");

            parser.next(); // var
            children.push_back(std::make_shared<VariableNode>(parser, this, attributes["init"]));
        } else if (next == "fun") {
            if (attributes["init"])
                throw ParseError(parser, "Internal type error, wrong init branch.");

            if (attributes["implicit"])
                throw ParseError(parser, "Attribute implicit used on variable declaration.");

            parser.next(); // fun
            children.push_back(std::make_shared<MethodNode>(parser, this, false, false));
        } else if (attributes["init"] && (next == "(" || next == "{")) { // constructor is special
            children.push_back(std::make_shared<MethodNode>(parser, this, true, attributes["implicit"]));
        } else if (next == "route") {
            if (type != Type::Page)
                throw ParseError(parser, "Route element can only be used on page components.");

            children.push_back(std::make_shared<RouteNode>(parser, this));
        } else if (next == "view" || next == "scene") {
            if (type != Type::Page)
                throw ParseError(parser, "Element with type {} can only be used on page components.", next);

            // expression nodes can be assumed to be UI view/scene nodes
            children.push_back(ExpressionNode::parse(parser, this));
        } else {
            throw ParseError(parser, "Unknown action character {}, in type {}, with name {}.", action, name, next);
        }
    }

    if (parser.next() != "}")
        throw ParseError(parser, "Internal type error, expected }}.");

    Node *firstInit = searchThis([](Node *node) { return node->type == Type::Method && node->as<MethodNode>()->init; });

    if (!firstInit) {
        children.push_back(std::make_shared<MethodNode>(this)); // default init
    }
}
