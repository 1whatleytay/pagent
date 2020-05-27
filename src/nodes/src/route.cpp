#include <nodes/route.h>

#include <nodes/string.h>

RouteNode::RouteNode(Parser &parser, Node *parent) : Node(parent, Type::Route) {
    if (parser.next() != "route")
        throw ParseError(parser, "Internal route error, expected route.");

    children.push_back(std::make_shared<StringNode>(parser, this));
}
