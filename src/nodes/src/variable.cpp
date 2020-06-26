#include <nodes/variable.h>

#include <nodes/typename.h>
#include <nodes/expression.h>

void VariableNode::verify() {
    searchHere([this](Node *node) {
        if (node == this)
            return false;

        if (name == getName(node))
            throw VerifyError("Duplicate name {} in scope.", name);

        return false;
    });

    evaluate();

    Node::verify();
}

bool VariableNode::isField() {
    return parent->type == Type::Type;
}

Node *VariableNode::defaultValue() {
    size_t index = hasExplicitType ? 1 : 0;

    if (children.size() > index)
        return children[index].get();
    else
        return nullptr;
}

Typename VariableNode::evaluate(std::vector<Node *> visited) {
    if (std::find(visited.begin(), visited.end(), this) != visited.end())
        throw VerifyError("Circular evaluation detected, related to variable {}.", name);
    visited.push_back(this);

    if (hasExplicitType)
        return children[0]->as<TypenameNode>()->content;
    else if (fake)
        return evaluator();
    else
        return children[0]->as<ExpressionNode>()->evaluate(visited);
}

VariableNode::VariableNode(std::string name, Node *parent)
    : Node(parent, Type::Variable), name(move(name)), fake(true) { }

VariableNode::VariableNode(Parser &parser, Node *parent, bool init, bool shared)
    : Node(parent, Type::Variable), init(init), shared(shared) {
    name = parser.next();

    if (parser.peek() == ":") {
        hasExplicitType = true;
        parser.next(); // :
        children.push_back(std::make_shared<TypenameNode>(parser, this));
    }

    if (parser.peek() == "=") {
        parser.next(); // =
        children.push_back(ExpressionNode::parse(parser, this));
    } else if (!hasExplicitType) { // no type
        throw ParseError(parser, "Variable must be given type using : if not initialized.");
    }
}
