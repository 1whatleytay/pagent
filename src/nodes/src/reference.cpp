#include <nodes/reference.h>

#include <nodes/code.h>
#include <nodes/type.h>
#include <nodes/method.h>
#include <nodes/variable.h>
#include <nodes/expression.h>

ReferenceNode *ReferenceNode::next() {
    size_t nextIndex = 0;
    // this is ridiculous
    if (hasCall)
        nextIndex += paramCount;
    if (hasContent)
        nextIndex += 1;

    if (children.size() > nextIndex)
        return children[nextIndex]->as<ReferenceNode>();

    return nullptr;
}

TypeNode *ReferenceNode::findType(Node *referenced) {
    Typename extendType;

    if (hasCall || hasContent) {
        extendType = referenced->as<MethodNode>()->evaluateReturn();
    } else {
        if (referenced->type == Type::Method)
            extendType = referenced->as<MethodNode>()->evaluate();
        else
            extendType = referenced->as<VariableNode>()->evaluate();
    }

    TypeNode *evalNode = referenced->searchScope([&extendType](Node *node) {
        return node->type == Type::Type && Typename(node->as<TypeNode>()->name) == extendType;
    })->as<TypeNode>();

    if (!evalNode)
        throw VerifyError("Could not find type with name {} to extend.", extendType.toString());

    return evalNode;
}

std::vector<Node *> ReferenceNode::dereference(Node *reference) {
    std::vector<Node *> result = dereferenceThis(reference);

    if (next()) {
        Node *evalNode = findType(selectFrom(result));

        return next()->dereference(evalNode);
    }

    return result;
}

std::vector<Node *> ReferenceNode::dereferenceThis(Node *reference) {
    std::vector<Node *> result;

    std::function<bool(Node *)> checker = [this, &result](Node *node) {
        // methods can also be referenced
        if (getName(node) == content)
            result.push_back(node);

        return false;
    };

    if (reference)
        reference->searchThis(checker);
    else
        searchScope(checker);

    if (result.empty())
        throw VerifyError("Reference {} does not reference anything.", content);

    // constructor calling
    if (result[0]->type == Type::Type) {
        if (result.size() != 1)
            throw VerifyError("Internal reference error, multiple constructor types.");

        TypeNode *typeNode = result[0]->as<TypeNode>();
        result.clear();

        typeNode->searchChildren([&result](Node *node) {
            if (node->type == Type::Method && node->as<MethodNode>()->init)
                result.push_back(node);

            return false;
        });
    }

    return result;
}

Node * ReferenceNode::select() {
    return selectFrom(dereference());
}

Node *ReferenceNode::selectFrom(const std::vector<Node *> &nodes) {
    if (nodes.empty())
        throw VerifyError("Node dereferenced to nothing.");

    if (nodes[0]->type == Type::Method && (hasCall || hasContent)) {
        std::vector<Node *> values = getParameters();

        for (Node *node : nodes) {
            if (node->type != Type::Method)
                throw VerifyError("Reference referred to method type but found non method type with same name.");

            std::vector<ssize_t> temp;

            bool matches = node->as<MethodNode>()->parameters().map(values, names, temp);

            if (matches)
                return node;
        }

        throw VerifyError("Could not find a matching method for {}.", content);
    } else {
        if (nodes.size() != 1)
            throw VerifyError("Node was Type or Variable but dereferenced to multiple nodes.");

        return nodes[0];
    }
}

Node *ReferenceNode::getContent() {
    if (hasContent) {
        return children[paramCount].get();
    }

    return nullptr;
}

std::vector<Node *> ReferenceNode::getParameters() {
    std::vector<Node *> values(paramCount);

    for (size_t a = 0; a < paramCount; a++) {
        values[a] = children[a].get();
    }

    return values;
}

Typename ReferenceNode::evaluate(std::vector<Node *> visited) {
    if (std::find(visited.begin(), visited.end(), this) != visited.end())
        throw VerifyError("Circular evaluation detected, related to reference {}.", content);
    visited.push_back(this);

    Node *node = select();

    switch (node->type) {
        case Type::Variable:
            return node->as<VariableNode>()->evaluate(visited);
        case Type::Method:
            if (!hasCall && !hasContent)
                return node->as<MethodNode>()->evaluate();
            else
                return node->as<MethodNode>()->evaluateReturn();
        case Type::Type:
            return node->as<TypeNode>()->evaluate();
        default:
            throw VerifyError("Reference referred to unknown type.");
    }
}

std::shared_ptr<Node> ReferenceNode::steal() {
    if (hasContent) {
        std::shared_ptr<Node> result = children[paramCount];

        hasContent = false;
        children.erase(children.begin() + paramCount);

        return result;
    }

    return nullptr;
}

void ReferenceNode::verify() {
    if (!select())
        throw VerifyError("Could not evaluate reference {}.", content);
}

ReferenceNode::ReferenceNode(Parser &parser, Node *parent) : Node(parent, Type::Reference) {
    content = parser.next();

    if (parser.peek() == "(") {
        hasCall = true;

        parser.next(); // (
        while (!parser.empty()) {
            if (parser.peek() == ")")
                break;

            std::string name = parser.next();
            if (parser.peek() == ":") {
                parser.next(); // :
                names[name] = children.size();
            } else {
                parser.rollback();
            }

            // sources
            children.push_back(ExpressionNode::parse(parser, this));
            paramCount++;

            if (parser.peek() == ",")
                parser.next(); // ,
            else if (parser.peek() != ")")
                throw ParseError(parser, "Arguments must be separated with commas.");
        }

        if (parser.next() != ")")
            throw ParseError(parser, "Expected ) after arguments list.");
    }

    if (parser.peek() == "{") {
        hasContent = true;

        parser.next(); // {

        children.push_back(std::make_shared<CodeNode>(parser, this));

        if (parser.next() != "}")
            throw ParseError(parser, "Expected } after call content.");
    }

    if (parser.peek() == ".") {
        parser.next(); // .

        children.push_back(std::make_shared<ReferenceNode>(parser, this));
    }
}
