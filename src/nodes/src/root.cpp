#include <nodes/root.h>

#include <nodes/enum.h>
#include <nodes/type.h>
#include <nodes/method.h>
#include <nodes/variable.h>

RootNode::RootNode(Parser &parser, Node *parent) : Node(parent, Type::Root) {
    while (!parser.empty()) {
        std::string next = parser.peek();

        if (next == "}") {
            if (!parent)
                throw ParseError(parser, "Unexpected }} in global context.");

            break;
        } else if (next == "enum") {
            children.push_back(std::make_shared<EnumNode>(parser, this));
        } else if (next == "type" || next == "page") {
            children.push_back(std::make_shared<TypeNode>(parser, this));
        } else if (next == "var") {
            parser.next(); // var
            children.push_back(std::make_shared<VariableNode>(parser, this, false));
        } else if (next == "fun") {
            parser.next(); // fun
            children.push_back(std::make_shared<MethodNode>(parser, this, false, false));
        } else {
            throw ParseError(parser, "Unknown keyword {}.", next);
        }
    }
}
