#include <nodes/for.h>

#include <nodes/code.h>
#include <nodes/variable.h>
#include <nodes/reference.h>
#include <nodes/expression.h>

bool ForNode::hasValue() {
    size_t resultIndex = op == Operator::In ? 2 : 1;

    return children[resultIndex]->type == Type::Expression;
}

ForNode::ForNode(Parser &parser, Node *parent) : Node(parent, Type::For) {
    if (parser.next() != "for")
        throw ParseError(parser, "Internal for error, expected for.");

    parser.select();

    // yeah this looks crazy but it basically says if either the next or the next next word is "in"
    if (parser.next() == "in" || parser.next() == "in") {
        op = Operator::In;
    }

    parser.back();

    if (op == Operator::In) {
        if (parser.peek() == "in") {
            throw ParseError(parser, "Expected variable name.");
//            children.push_back(std::make_shared<VariableNode>("it", this));
        } else {
            children.push_back(std::make_shared<VariableNode>(parser.next(), this));
        }

        if (parser.next() != "in")
            throw ParseError(parser, "Internal for error, expected in.");

        children.push_back(ExpressionNode::parse(parser, this));

        // evaluator has to be set for the first fake variable node or else hell breaks lose
        children[0]->as<VariableNode>()->evaluator = [this]() {
            // iterate more than just arrays i suppose
            Typename arrayType = children[1]->as<ExpressionNode>()->evaluate();

            if (!arrayType.array)
                throw VerifyError("For in loop expression evaluates to a non iterable.");

            return arrayType.children[0];
        };
    } else { // while
        children.push_back(ExpressionNode::parse(parser, this));
    }

    // body
    if (parser.peek() == "-") {
        parser.next(); // -

        if (parser.next() != ">")
            throw ParseError(parser, "Expected for body to be declared with ->.");

        children.push_back(ExpressionNode::parse(parser, this));
    } else {
        if (parser.peek() == "{") {
            parser.next(); // {

            children.push_back(std::make_shared<CodeNode>(parser, this));

            if (parser.next() != "}")
                throw ParseError(parser, "Expected for body to be declared with }}.");
        } else {
            // sometimes reference nodes capture the if body in content calls
            ExpressionNode *rightmost = children[children.size() - 1].get()->as<ExpressionNode>();

            // keep on going until we find a reference literal node
            while (rightmost->op != ExpressionNode::Operator::Literal) {
                rightmost = rightmost->children[rightmost->children.size() - 1]->as<ExpressionNode>();
            }

            Node *contentNode = rightmost->children[0].get();

            if (contentNode->type != Type::Reference)
                throw ParseError(parser, "Expected {{ for for body.");

            std::shared_ptr<Node> body = contentNode->as<ReferenceNode>()->steal();

            if (!body)
                throw ParseError(parser, "Expected {{ for for body.");

            children.push_back(body);
        }
    }
}
