#include <target/js/root.h>

#include <nodes/type.h>

std::string indent(const std::string &content) {
    // i cant be bothered to https://stackoverflow.com/a/3418285
    if (content.empty())
        return "";

    std::string result = "\t" + content;

    size_t start_pos = 0;
    while((start_pos = result.find("\n", start_pos)) != std::string::npos) {
        result.replace(start_pos, 1, "\n\t");
        start_pos += 2;
    }

    return result;
}

size_t JsRoot::nextId() {
    return id++;
}

std::string JsRoot::build() {
    std::stringstream body;

    for (const auto &e : types) {
        body << e.second->build();
    }
    for (const auto &e : variables) {
        body << e.second->build();
    }
    for (const auto &e : functions) {
        body << e.second->build();
    }

    return body.str();
}

JsRoot::JsRoot(RootNode *node) {
    // load standard
    Node *root = node->searchParents([](Node *node) { return node->parent == nullptr; });
    view = node->searchChildren([](Node *node) {
        return node->type == Node::Type::Type && node->as<TypeNode>()->name == "view";
    });
    scene = node->searchChildren([](Node *node) {
        return node->type == Node::Type::Type && node->as<TypeNode>()->name == "scene";
    });
    text = node->searchChildren([](Node *node) {
        return node->type == Node::Type::Type && node->as<TypeNode>()->name == "Text";
    });
    button = node->searchChildren([](Node *node) {
        return node->type == Node::Type::Type && node->as<TypeNode>()->name == "Button";
    });

    for (const auto &child : node->children) {
        if (child.get() == view || child.get() == scene || child.get() == text || child.get() == button)
            continue;

        switch (child->type) {
            case Node::Type::Type:
                types[child->as<TypeNode>()] = std::make_shared<JsType>(*this, child->as<TypeNode>());
                break;
            case Node::Type::Method:
                functions[child->as<MethodNode>()] = std::make_shared<JsFunction>(*this, child->as<MethodNode>());
                break;
            case Node::Type::Variable:
                variables[child->as<VariableNode>()] = std::make_shared<JsAction>(*this, child->as<VariableNode>());
                break;
            default:
                assert(false);
        }
    }
}
