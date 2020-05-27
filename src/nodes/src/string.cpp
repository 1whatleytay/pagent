#include <nodes/string.h>

#include <nodes/expression.h>

#include <sstream>

StringNode::StringNode(Parser &parser, Node *parent) : Node(parent, Type::String) {
    std::string self = parser.next(); // quote

    if (self != "\"" && self != "\'")
        throw ParseError(parser, "Internal string error, quotes must be \" or \'.");

    std::stringstream output;
    parser.original = true;

    while (!parser.empty()) {
        output << parser.until({ "\\", self });
        std::string next = parser.next();
        if (next == self) {
            break;
        } else if (next == "\\") {
            std::string nextSymbol = parser.next();
            if (nextSymbol == "(") {
                indices.push_back(output.str().size());
                children.push_back(ExpressionNode::parse(parser, parent));

                if (parser.next() != ")")
                    throw std::runtime_error("Unknown error related to string templates.");
            } else if (nextSymbol == "'") {
                output << "'";
            } else {
                throw std::runtime_error("Unknown error related to string escapes.");
            }
        }
    }

    parser.original = false;

    text = output.str();
}
