#include <nodes/method.h>

#include <nodes/code.h>
#include <nodes/type.h>
#include <nodes/typename.h>
#include <nodes/variable.h>
#include <nodes/statement.h>
#include <nodes/attributes.h>
#include <nodes/expression.h>

void Parameters::add(Node *node) {
    VariableNode *var = node->as<VariableNode>();
    Typename varType = var->evaluate();

    parameters.push_back({ varType, var->name, node });
}

ssize_t Parameters::find(const std::string &name) {
    for (size_t a = 0; a < parameters.size(); a++) {
        if (parameters[a].name == name)
            return a;
    }

    return -1;
}

// this is going to brainf**k me
bool Parameters::map(std::vector<Node *> values, std::map<std::string, size_t> names, std::vector<ssize_t> &result) {
    // empty return on no match, but also empty return if there are no expressions but it is a valid match
    result.resize(values.size(), -1);

    std::vector<bool> usedParameter(parameters.size());

    for (const auto &name : names) {
        ssize_t index = find(name.first);

        if (index == -1)
            return false;

        Typename expType = values[name.second]->as<ExpressionNode>()->evaluate();

        if (expType != parameters[index].type)
            return false; // does not match
//            throw VerifyError("Parameter with name {} and type {} is incompatible with type {}.",
//                parameters[index].name, parameters[index].type.toString(), expType.toString());

        result[name.second] = index;
        usedParameter[index] = true;
    }

    for (size_t a = 0; a < result.size(); a++) {
        if (result[a] != -1)
            continue;

        size_t nextParameter;
        for (nextParameter = 0; nextParameter < usedParameter.size(); nextParameter++) {
            if (!usedParameter[nextParameter])
                break;
        }

        if (nextParameter >= usedParameter.size())
            return false; // does not match
//            throw VerifyError("Too many expressions in method call.");

        Typename expType = values[a]->as<ExpressionNode>()->evaluate();

        if (expType != parameters[nextParameter].type)
            return false; // does not match

        result[a] = nextParameter;
        usedParameter[nextParameter] = true;
    }

    for (size_t a = 0; a < usedParameter.size(); a++) {
        if (!usedParameter[a]) {
            VariableNode *var = parameters[a].reference->as<VariableNode>();

            if (var->children.empty() && !var->evaluate().optional)
                return false; // does not match
        }
    }

    return true;
}

Parameters MethodNode::parameters() {
    Parameters result;

    Node *parentType = nullptr;
    if (init) {
        parentType = searchParents([](Node *node) { return node->type == Type::Type; });

        if (!parentType)
            throw VerifyError("Internal method error, constructor does not have parent type.");
    }

    if (parentType) {
        parentType->searchHere([&result](Node *node) {
            if (node->type == Type::Variable) {
                VariableNode *var = node->as<VariableNode>();

                if (var->children.empty() && !var->evaluate().optional)
                    result.add(node);
            }

            return false;
        });
    }

    for (size_t a = 0; a < paramCount; a++) {
        result.add(children[a].get());
    }

    if (parentType) {
        parentType->searchHere([&result](Node *node) {
            if (node->type == Type::Variable) {
                VariableNode *var = node->as<VariableNode>();

                if (!var->children.empty() || var->evaluate().optional)
                    result.add(node);
            }

            return false;
        });
    }

    return result;
}

void MethodNode::verify() {
    // types of parameters have to exist!!!
    for (size_t a = 0; a < paramCount; a++) {
        Typename varType = children[a]->as<VariableNode>()->evaluate();

        if (varType == Typename::number || varType == Typename::boolean || varType == Typename::string) {
            continue; // these are okay, do not need to be declared
        }

        Node *node = searchScope([varType](Node *node) {
            return node->type == Type::Type && varType == Typename(node->as<TypeNode>()->name);
        });

        if (!node)
            throw VerifyError("In method {}, parameter declared with unknown type {}.", name, varType.toString());
    }

    searchHere([this](Node *node) {
        if (node == this)
            return false;

        if (name == getName(node)) {
            if (node->type == Type::Method) { // check params, due to overloading
                auto *method = node->as<MethodNode>();

                size_t thisIndex = 0;
                size_t thatIndex = 0;

                while (thisIndex < paramCount && thatIndex < method->paramCount) {
                    auto *thisParam = children[thisIndex]->as<VariableNode>();
                    auto *thatParam = method->children[thatIndex]->as<VariableNode>();

                    if (thisParam->evaluate().optional) {
                        thisIndex++;
                        continue;
                    }

                    if (thatParam->evaluate().optional) {
                        thatIndex++;
                        continue;
                    }

                    // first child must exist in method expression
                    if (thisParam->children[0]->as<TypenameNode>()->content
                        != thatParam->children[0]->as<TypenameNode>()->content)
                        break;

                    thisIndex++;
                    thatIndex++;
                }

                if (thisIndex == paramCount && thatIndex == method->paramCount)
                    throw VerifyError("Duplicate methods {} with identical signatures.", init ? "init" : name);
            } else {
                if (!init)
                    throw VerifyError("Duplicate name {} in scope.", name);
            }
        }

        // make sure parameters aren't the same

        return false;
    });

    Node::verify();
}

Typename MethodNode::evaluate() {
    Typename result;

    result.function = true;
    result.paramCount = paramCount;

    for (size_t a = 0; a < paramCount; a++) {
        result.children.push_back(children[a]->as<VariableNode>()->evaluate());
    }

    return result;
}

Typename MethodNode::evaluateReturn() {
    if (hasReturnType) {
        return children[paramCount]->as<TypenameNode>()->content;
    } else if (init) {
        Node *typeParent = searchParents([](Node *node) { return node->type == Type::Type; });

        if (!typeParent)
            throw VerifyError("No parent node for initializer constructor.");

        return typeParent->as<TypeNode>()->evaluate();
    } else {
        return Typename::null; // maybe error?
    }
}

MethodNode::MethodNode(Node *parent) : Node(parent, Type::Method), init(true) {
    // default init
    children.push_back(std::make_shared<CodeNode>(this)); // empty body
}

MethodNode::MethodNode(Parser &parser, Node *parent, bool init, bool implicit)
    : Node(parent, Type::Method), init(init), implicit(implicit) {
//    assert(!implicit);

    if (!init)
        name = parser.next();

    if (implicit && !init)
        throw ParseError(parser, "Implicit used on a regular method {}.", name);

    if (parser.peek() == "(") {
        parser.next(); // (

        while (!parser.empty()) {
            if (parser.peek() == ")")
                break;

            children.push_back(std::make_shared<VariableNode>(parser, this, false));
            paramCount++;

            // require!
            if (parser.peek() == ",")
                parser.next(); // ,
            else if (parser.peek() != ")")
                throw ParseError(parser, "Variable declarations must be separated with commas.");
        }

        if (parser.next() != ")")
            throw ParseError(parser, "Internal method error, expected ).");
    }

    if (parser.peek() == ":") {
        parser.next(); // :
        hasReturnType = true;
        children.push_back(std::make_shared<TypenameNode>(parser, this));
    }

    if (parser.peek() == "-") {
        parser.next(); // =

        if (parser.next() != ">")
            throw ParseError(parser, "Expected => return symbol but only got =.");

        children.push_back(ExpressionNode::parse(parser, this));
    } else if (parser.next() == "{") {
        children.push_back(std::make_shared<CodeNode>(parser, this));

        if (parser.next() != "}")
            throw ParseError(parser, "Internal method error, expected }}.");
    } else {
        throw ParseError(parser, "Expected {{ after function parameters.");
    }
}
