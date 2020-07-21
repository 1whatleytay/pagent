#include <nodes/function.h>

#include <nodes/code.h>
#include <nodes/type.h>
#include <nodes/typename.h>
#include <nodes/variable.h>
#include <nodes/attributes.h>
#include <nodes/expression.h>

std::vector<Parameter> styleParameters = {
    { Typename("Color").asOptional(), "color" },
    { Typename("Align").asOptional(), "align" },
    { Typename("ContentAlign").asOptional(), "contentAlign" },
    { Typename::number.asOptional(), "width" },
    { Typename::number.asOptional(), "height" },
    { Typename::number.asOptional(), "column" },
    { Typename::number.asOptional(), "row" },
    { Typename("Padding").asOptional(), "padding" },
    { Typename("Margin").asOptional(), "margin" },
    { Typename::number.asOptional(), "rounded" },
    { Typename::boolean.asOptional(), "stretch" }, // flex: 1 lol
    { Typename("Border").asOptional(), "border" },
    { Typename(std::vector<Typename> { }).asOptional(), "click" },
    { Typename(std::vector<Typename> { }).asOptional(), "mouseEnter" },
    { Typename(std::vector<Typename> { }).asOptional(), "mouseExit" },
};

MapResult::MapResult(std::string error) : matches(false), error(move(error)) { }

void Parameters::add(Node *node) {
    VariableNode *var = node->as<VariableNode>();
    Typename varType = var->evaluate();

    push_back({ varType, var->name, node });
}

ssize_t Parameters::find(const std::string &name) {
    for (size_t a = 0; a < size(); a++) {
        if (operator[](a).name == name)
            return a;
    }

    return -1;
}

// this is going to brainf**k me
MapResult Parameters::map(std::vector<Node *> values, std::map<std::string, size_t> names) {
    MapResult result;

    // empty return on no match, but also empty return if there are no expressions but it is a valid match
    result.map.resize(values.size(), -1);

    std::vector<bool> usedParameter(size());

    for (const auto &name : names) {
        ssize_t index = find(name.first);

        if (index == -1)
            return MapResult(fmt::format("No parameter named {}.", name.first));

        Typename expType = values[name.second]->as<ExpressionNode>()->evaluate();

        if (expType != operator[](index).type)
            return MapResult(fmt::format("Expression with type {} does not match parameter {} with type {}.",
                expType.toString(), operator[](index).name, operator[](index).type.toString()));

        result.map[name.second] = index;
        usedParameter[index] = true;
    }

    for (size_t a = 0; a < values.size(); a++) {
        if (result.map[a] != -1)
            continue;

        size_t nextParameter;
        for (nextParameter = 0; nextParameter < usedParameter.size(); nextParameter++) {
            if (!usedParameter[nextParameter])
                break;
        }

        if (nextParameter >= usedParameter.size())
            return MapResult(fmt::format("Too many parameters, {} expected, {} passed.", size(), values.size()));

        Typename expType = values[a]->as<ExpressionNode>()->evaluate();

        if (expType != operator[](nextParameter).type)
            return MapResult(fmt::format("Expression with type {} does not match parameter {} with type {}.",
                expType.toString(), operator[](nextParameter).name, operator[](nextParameter).type.toString()));

        result.map[a] = nextParameter;
        usedParameter[nextParameter] = true;
    }

    for (size_t a = 0; a < usedParameter.size(); a++) {
        if (!usedParameter[a]) {
            bool optional = operator[](a).type.optional;

            // builtins better be optional
            if (operator[](a).reference) {
                VariableNode *var = operator[](a).reference->as<VariableNode>();

                if (var->children.empty() && !optional)
                    return MapResult(fmt::format("Missing value for non-optional parameter {}.", operator[](a).name));
            } else if (!optional) {
                return MapResult(fmt::format("Missing value for non-optional parameter {}.", operator[](a).name));
            }
        }
    }

    return result;
}

bool FunctionNode::isMethod() {
    return parent->type == Type::Type;
}

Parameters FunctionNode::parameters() {
    Parameters result;

    TypeNode *parentType = nullptr;
    if (init) {
        parentType = searchParents([](Node *node) { return node->type == Type::Type; })->as<TypeNode>();

        if (!parentType)
            throw VerifyError("Internal method error, constructor does not have parent type.");
    }

    if (parentType) {
        parentType->searchChildren([&result](Node *node) {
            if (node->type == Type::Variable) {
                VariableNode *var = node->as<VariableNode>();

                if (var->init && (var->children.empty() && !var->evaluate().optional))
                    result.add(node);
            }

            return false;
        });
    }

    for (size_t a = 0; a < paramCount; a++) {
        result.add(children[a].get());
    }

    if (parentType) {
        parentType->searchChildren([&result](Node *node) {
            if (node->type == Type::Variable) {
                VariableNode *var = node->as<VariableNode>();

                if (var->init && (!var->children.empty() || var->evaluate().optional))
                    result.add(node);
            }

            return false;
        });

        if (parentType->isPage) {
            result.insert(result.end(), styleParameters.begin(), styleParameters.end());
        }
    }

    return result;
}

void FunctionNode::verify() {
    // types of parameters have to exist!!!
//    for (size_t a = 0; a < paramCount; a++) {
//        Typename varType = children[a]->as<VariableNode>()->evaluate();
//
//        if (varType == Typename::number || varType == Typename::boolean || varType == Typename::string) {
//            continue; // these are okay, do not need to be declared
//        }
//
//        Node *node = searchScope([varType](Node *node) {
//            return node->type == Type::Type && varType == Typename(node->as<TypeNode>()->name);
//        });
//
//        if (!node)
//            throw VerifyError("In method {}, parameter declared with unknown type {}.", name, varType.toString());
//    }

    searchHere([this](Node *node) {
        if (node == this)
            return false;

        if (name == getName(node)) {
            if (node->type == Type::Function) { // check params, due to overloading
                auto *method = node->as<FunctionNode>();

                size_t thisIndex = 0;
                size_t thatIndex = 0;

                while (thisIndex < paramCount && thatIndex < method->paramCount) {
                    auto *thisParam = children[thisIndex]->as<VariableNode>();
                    auto *thatParam = method->children[thatIndex]->as<VariableNode>();

                    Typename thisType = thisParam->evaluate();
                    Typename thatType = thatParam->evaluate();

                    if (thisType.optional) {
                        thisIndex++;
                        continue;
                    }

                    if (thatType.optional) {
                        thatIndex++;
                        continue;
                    }

                    // first child must exist in method expression
                    if (thisType != thatType)
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

Typename FunctionNode::evaluate() {
    Typename result;

    result.function = true;
    result.paramCount = paramCount;

    for (size_t a = 0; a < paramCount; a++) {
        result.children.push_back(children[a]->as<VariableNode>()->evaluate());
    }

    return result;
}

Typename FunctionNode::evaluateReturn() {
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

Node *FunctionNode::body() {
    return children[paramCount + hasReturnType].get();
}

FunctionNode::FunctionNode(Node *parent) : Node(parent, Type::Function), init(true) {
    // default init
    children.push_back(std::make_shared<CodeNode>(this)); // empty body
}

FunctionNode::FunctionNode(Parser &parser, Node *parent, bool init, bool implicit)
    : Node(parent, Type::Function), init(init), implicit(implicit) {
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
