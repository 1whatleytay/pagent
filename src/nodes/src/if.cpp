#include <nodes/if.h>

#include <nodes/code.h>
#include <nodes/reference.h>
#include <nodes/expression.h>

bool IfNode::hasValue() {
    return children[1]->type == Type::Expression;
}

IfNode::IfNode(Parser &parser, Node *parent) : Node(parent, Type::If) {
    if (parser.next() != "if")
        throw ParseError(parser, "Internal if error, expected if.");

    children.push_back(ExpressionNode::parse(parser, this));

    if (parser.peek() == "-") {
        parser.next(); // -

        if (parser.next() != ">")
            throw ParseError(parser, "Expected -> for if body.");

        children.push_back(ExpressionNode::parse(parser, this));
    } else {
        if (parser.peek() == "{") {
            parser.next(); // {

            children.push_back(std::make_shared<CodeNode>(parser, this));

            if (parser.next() != "}")
                throw ParseError(parser, "Expected }} to close if body.");
        } else {
            // sometimes reference nodes capture the if body in content calls
            ExpressionNode *rightmost = children[0].get()->as<ExpressionNode>();

            // keep on going until we find a reference literal node
            while (rightmost->op != ExpressionNode::Operator::Literal) {
                rightmost = rightmost->children[rightmost->children.size() - 1]->as<ExpressionNode>();
            }

            Node *contentNode = rightmost->children[0].get();

            if (contentNode->type != Type::Reference)
                throw ParseError(parser, "Expected {{ for if body.");

            std::shared_ptr<Node> body = contentNode->as<ReferenceNode>()->steal();

            if (!body)
                throw ParseError(parser, "Expected {{ for if body.");

            children.push_back(body);
        }
    }
}
