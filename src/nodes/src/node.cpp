#include <nodes/node.h>

#include <nodes/enum.h>
#include <nodes/type.h>
#include <nodes/method.h>
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
        /* && optional == value.optional*/;
}

bool Typename::operator!=(const Typename &value) const {
    return !operator==(value);
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

Typename::Typename(std::string name, bool optional) : name(move(name)), optional(optional) { }

std::string getName(Node *node) {
    switch (node->type) {
        case Node::Type::Enum:
            return node->as<EnumNode>()->name;
        case Node::Type::Type:
            return node->as<TypeNode>()->name;
        case Node::Type::Page:
            return node->as<TypeNode>()->name;
        case Node::Type::Method:
            return node->as<MethodNode>()->name;
        case Node::Type::Variable:
            return node->as<VariableNode>()->name;
        default:
            return "";
    }
}
