#include <nodes/typename.h>

TypenameNode::TypenameNode(Parser &parser, Node *parent) : Node(parent, Type::Typename) {
    if (parser.peek() == "[") {
        parser.next(); // [

        content.array = true;

        children.push_back(std::make_shared<TypenameNode>(parser, this));
        content.children.push_back(children[0]->as<TypenameNode>()->content);

        if (parser.next() != "]")
            throw ParseError(parser, "Expected ] after array type.");
    } else {
        if (parser.peek() == "Function") {
            parser.next(); // Function

            content.function = true;

            if (parser.peek() == "(") {
                parser.next(); // (

                while (!parser.empty()) {
                    if (parser.peek() == ")")
                        break;

                    children.push_back(std::make_shared<TypenameNode>(parser, this));
                    content.children.push_back(children[content.paramCount]->as<TypenameNode>()->content);
                    content.paramCount++;

                    if (parser.peek() == ",")
                        parser.next(); // ,
                    else if (parser.peek() != ")")
                        throw ParseError(parser, "Variable declarations must be separated with commas.");
                }

                if (parser.next() != ")")
                    throw ParseError(parser, "Internal function error, expected ).");
            }
        } else {
            content.name = parser.next();
        }

        if (parser.peek() == "?") {
            parser.next();
            content.optional = true;
        }

        // return type
        if (content.function && parser.peek() == ":") {
            parser.next(); // :

            children.push_back(std::make_shared<TypenameNode>(parser, this));
            content.children.push_back(children[content.paramCount]->as<TypenameNode>()->content);
        }
    }
}
