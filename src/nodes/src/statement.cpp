#include <nodes/statement.h>

#include <nodes/function.h>
#include <nodes/variable.h>
#include <nodes/expression.h>

void StatementNode::verify() {
    switch (op) {
        case Operator::Return: {
            if (!children.empty()) {
                Node *node = parent;

                while (node && node->type != Type::Function)
                    node = node->parent;

                if (!node)
                    throw VerifyError("Internal statement error, could not find parent method for return.");

                auto *method = node->as<FunctionNode>();

                if (!method->hasReturnType)
                    throw VerifyError("Internal statement error, return has expression but method returns nothing.");

                Typename returnType = method->children[method->paramCount]->as<TypenameNode>()->content;

                auto *expression = children[0]->as<ExpressionNode>();
                Typename returnValue = expression->evaluate();

                if (returnValue != returnType) {
                    throw VerifyError("Method {} has return type {} but returns {}.",
                        method->name, returnType.toString(), returnValue.toString());
                }
            }

            break;
        }
        default:
            break;
    }

    Node::verify();
}

StatementNode::StatementNode(Parser &parser, Node *parent) : Node(parent, Type::Statement) {
    std::string start = parser.peek();

    if (start == "return") {
        op = Operator::Return;
        parser.next(); // return

        Node *node = parent;

        while (node && node->type != Type::Function)
            node = node->parent;

        if (!node)
            throw ParseError(parser, "Internal statement error, could not find parent method for return.");

        if (node->as<FunctionNode>()->hasReturnType)
            children.push_back(ExpressionNode::parse(parser, this));

        return;
    } else if (start == "break") {
        op = Operator::Break;
    } else if (start == "continue") {
        op = Operator::Continue;
    }
}
