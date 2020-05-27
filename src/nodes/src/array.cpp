#include <nodes/array.h>

#include <nodes/expression.h>

Typename ArrayNode::evaluate(std::vector<Node *> visited) {
    if (std::find(visited.begin(), visited.end(), this) != visited.end())
        throw VerifyError("Circular evaluation detected.");
    visited.push_back(this);

    Typename result;

    result.array = true;

    if (children.empty())
        result.children.push_back(Typename::empty);
    else
        result.children.push_back(children[0]->as<ExpressionNode>()->evaluate(visited));

    return result;
}

ArrayNode::ArrayNode(Parser &parser, Node *parent) : Node(parent, Type::Array) {
    if (parser.next() != "[")
        throw ParseError(parser, "Internal array error, expected [.");

    while (!parser.empty()) {
        if (parser.peek() == "]")
            break;

        children.push_back(std::make_shared<ExpressionNode>(parser, this));

        if (parser.peek() == ",")
            parser.next();
    }

    if (parser.next() != "]")
        throw ParseError(parser, "Unexpected end of file in middle of array literal.");
}
