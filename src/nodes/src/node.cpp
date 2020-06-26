#include <nodes/node.h>

#include <nodes/enum.h>
#include <nodes/type.h>
#include <nodes/enumname.h>
#include <nodes/function.h>
#include <nodes/variable.h>

const Typename Typename::any = Typename("Any");
const Typename Typename::empty = Typename("Empty");
const Typename Typename::null = Typename("Null");
const Typename Typename::string = Typename("String");
const Typename Typename::number = Typename("Number");
const Typename Typename::boolean = Typename("Boolean");

bool Typename::operator==(const Typename &value) const {
    // do not compare optional! optional decoding is implicit so it would be tough to let this through
    return name == value.name
           && array == value.array
           && function == value.function
           && children == value.children
           && paramCount == value.paramCount
        /* && optional == value.optional*/;
}

bool Typename::operator!=(const Typename &value) const {
    return !operator==(value);
}

Typename Typename::asOptional(bool value) const {
    Typename result = *this;
    result.optional = value;
    return result;
}

std::string Typename::toString() const {
    if (function) {
        std::vector<std::string> params(paramCount);
        for (size_t a = 0; a < paramCount; a++) {
            params[a] = children[a].toString();
        }

        return fmt::format("Function({}){}{}", fmt::join(params, ", "),
            children.size() != paramCount ? fmt::format(": {}", children[paramCount].toString()) : "",
            optional ? "?" : "");
    } else if (array) {
        return fmt::format("[{}]{}", children[0].toString(), optional ? "?" : "");
    } else {
        return fmt::format("{}{}", name, optional ? "?" : "");
    }
}

Typename::Typename(std::string name) : name(move(name)) { }
Typename::Typename(std::vector<Typename> params)
    : paramCount(params.size()), children(move(params)), function(true) { }
Typename::Typename(std::vector<Typename> params, Typename returnType)
    : paramCount(params.size() + 1), children(move(params)), function(true) {
    children.push_back(returnType);
}

std::string getName(Node *node) {
    switch (node->type) {
        case Node::Type::Enum:
            return node->as<EnumNode>()->name;
        case Node::Type::Type:
            return node->as<TypeNode>()->name;
        case Node::Type::Enumname:
            return node->as<EnumnameNode>()->name;
        case Node::Type::Function:
            return node->as<FunctionNode>()->name;
        case Node::Type::Variable:
            return node->as<VariableNode>()->name;
        default:
            return "";
    }
}
