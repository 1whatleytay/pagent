#include <nodes/code.h>

#include <nodes/comment.h>
#include <nodes/variable.h>
#include <nodes/statement.h>
#include <nodes/attributes.h>
#include <nodes/expression.h>

CodeNode::CodeNode(Node *parent) : Node(parent, Type::Code) { }

CodeNode::CodeNode(Parser &parser, Node *parent) : Node(parent, Type::Code) {
    while (!parser.empty()) {
        if (parser.peek() == "}")
            break;

        std::string next = parser.peek();

        if (next == "var") {
            parser.next(); // var
            children.push_back(std::make_shared<VariableNode>(parser, this, false, false));
        } else if (next == "return" || next == "break" || next == "continue") {
            children.push_back(std::make_shared<StatementNode>(parser, this));
        } else {
            size_t point = parser.select();

            // working in comments T_T
            if (parser.next() == "/" && parser.next() == "/") {
                parser.jump(point);
                children.push_back(std::make_shared<CommentNode>(parser, this));
            } else {
                parser.jump(point);

                if (parser.next() == "/" && parser.next() == "*") {
                    parser.jump(point);
                    children.push_back(std::make_shared<CommentNode>(parser, this));
                } else {
                    parser.jump(point);
                    children.push_back(ExpressionNode::parse(parser, this));
                }
            }
        }
    }
}
