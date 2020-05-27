#include <nodes/lambda.h>

#include <nodes/code.h>
#include <nodes/variable.h>
#include <nodes/expression.h>

Typename LambdaNode::evaluate() {
    Typename result;

    result.function = true;
    result.paramCount = paramCount;

    for (size_t a = 0; a < paramCount; a++) {
        result.children.push_back(children[a]->as<VariableNode>()->evaluate());
    }

    return result;
}

LambdaNode::LambdaNode(Parser &parser, Node *parent) : Node(parent, Type::Lambda) {
    // lambda, identified by `{` or `()` or `(* :`
    if (parser.peek() == "(") {
        parser.next(); // (

        while (!parser.empty()) {
            if (parser.peek() == ")")
                break;

            children.push_back(std::make_shared<VariableNode>(parser, this, false));
            paramCount++;

            if (parser.peek() == ",")
                parser.next(); // ,
            else if (parser.peek() != ")")
                throw ParseError(parser, "Variable declarations must be separated with commas.");
        }

        if (parser.next() != ")")
            throw ParseError(parser, "Internal method error, expected ).");

        if (parser.peek() == "-") { // ->
            parser.next(); // -

            if (parser.next() != ">")
                throw ParseError(parser, "Expected => lambda signature, but just got =.");

            if (parser.peek() == "{")
                throw ParseError(parser,
                    "Lambda returning another lambda might be a mistake. Remove -> or add () to make it explicit.");

            children.push_back(ExpressionNode::parse(parser, this));
        } else if (parser.next() == "{") { // { }
            children.push_back(std::make_shared<CodeNode>(parser, this));

            if (parser.next() != "}")
                throw ParseError(parser, "Expected } after lambda content.");
        } else {
            throw ParseError(parser, "Expected => or { for lambda definition but got neither.");
        }
    } else if (parser.peek() == "{") {
        parser.next(); // {

        children.push_back(std::make_shared<CodeNode>(parser, this));

        if (parser.next() != "}")
            throw ParseError(parser, "Expected } at end of lambda.");

        return;
    }
}
