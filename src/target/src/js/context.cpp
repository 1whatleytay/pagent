#include <target/js/context.h>

#include <nodes/enum.h>
#include <nodes/type.h>
#include <nodes/route.h>
#include <nodes/string.h>
#include <nodes/function.h>

//#define DONT_BASE

std::string JsContext::reserveName(Node *node) {
    // this should be scoped but whatever
    std::string base = getName(node);
    if (node->type == Node::Type::Function && node->as<FunctionNode>()->init)
        base = "$build";
    if ((node->type == Node::Type::Variable && node->as<VariableNode>()->shared)
        || node->type == Node::Type::Enumname) {
        base = fmt::format("{}${}", reserveName(node->parent), base);
    }
    assert(!base.empty());

    auto &scopeNames = names[node->parent];

    auto existingName = scopeNames.find(node);
    if (existingName != scopeNames.end())
        return existingName->second;

#ifdef DONT_BASE
    std::string tryName = fmt::format("{}${}", base, nextId());
#else
    std::string tryName = base;
#endif

    while (true) {
        bool fail = false;
        for (const auto &pair : scopeNames) {
            if (pair.second == tryName) {
                fail = true;
                break;
            }
        }

        if (!fail)
            break;

        tryName = fmt::format("{}${}", base, nextId());
    }

    scopeNames[node] = tryName;

    return tryName;
}

std::string JsContext::indent(const std::string &content) {
    // i cant be bothered to https://stackoverflow.com/a/3418285
    if (content.empty())
        return "";

    std::string result = indentSpace + content;

    size_t start_pos = 0;
    while((start_pos = result.find("\n", start_pos)) != std::string::npos) {
        result.replace(start_pos, 1, "\n" + indentSpace);
        start_pos += 3;
    }

    return result;
}

size_t JsContext::nextId() {
    return id++;
}

std::string JsContext::build() {
    std::stringstream body;

    std::vector<Node *> noBuild = {
        view,
        scene,
        text,
        button,
        icon,
        image,
        link,
        clickable,
        array,
        align,
        contentAlign
    };

    for (const auto &child : root->children) {
        if (std::find(noBuild.begin(), noBuild.end(), child.get()) != noBuild.end())
            continue;

        switch (child->type) {
            case Node::Type::Type:
                body << genType(child->as<TypeNode>());
                break;

            case Node::Type::Function:
                body << genFunction(child->as<FunctionNode>());
                break;

            case Node::Type::Variable:
                body << genVariable(child->as<VariableNode>());
                break;

            case Node::Type::Enum:
                body << genEnum(child->as<EnumNode>());
                break;

            case Node::Type::Comment:
                break; // OK

            default:
                assert(false);
        }
    }

    std::stringstream routeStream;

    root->searchThis([&routeStream](Node *node) {
        if (node->type == Node::Type::Route) {
            RouteNode *route = node->as<RouteNode>();
            TypeNode *parent = route->parent->as<TypeNode>();
            StringNode *string = route->children[0]->as<StringNode>();

            assert(string->indices.empty()); // TODO: implement

            // TODO: proper jsReferences to parent->name
            routeStream << fmt::format("\nif ($path == '{}') {{ return new {}() }}", string->text, parent->name);
        }

        return false;
    });

    std::string routeMethod = routeStream.str();
    if (!routeMethod.empty()) {
        routeMethod += "\nreturn new $Component()";

        body << "\nfunction $route($path) {" << indent(routeMethod) << "\n}";
    }

    return body.str();
}

NodeChecker findTypeWithName(std::string name) {
    return [name](Node *node) {
        return node->type == Node::Type::Type && node->as<TypeNode>()->name == name;
    };
}

NodeChecker findFunctionWithName(std::string name) {
    return [name](Node *node) {
        return node->type == Node::Type::Function && node->as<FunctionNode>()->name == name;
    };
}

NodeChecker findInitVarWithName(std::string name) {
    return [name](Node *node) {
        if (node->type != Node::Type::Variable)
            return false;

        VariableNode *e = node->as<VariableNode>();

        return e->init && e->name == name;
    };
}

NodeChecker findEnumWithName(std::string name) {
    return [name](Node *node) {
        return node->type == Node::Type::Enum && node->as<EnumNode>()->name == name;
    };
}

JsContext::JsContext(RootNode *root) : root(root) {
    // load standard
    view = root->searchChildren(findTypeWithName("View"));
    scene = root->searchChildren(findTypeWithName("Scene"));
    text = root->searchChildren(findTypeWithName("Text"));
    button = root->searchChildren(findTypeWithName("Button"));
    image = root->searchChildren(findTypeWithName("Image"));
    icon = root->searchChildren(findTypeWithName("Icon"));
    clickable = root->searchChildren(findTypeWithName("Clickable"));
    link = root->searchChildren(findTypeWithName("Link"));
    array = root->searchChildren(findTypeWithName("Array"));
    align = root->searchChildren(findEnumWithName("Align"));
    contentAlign = root->searchChildren(findEnumWithName("ContentAlign"));

    textStyleTextSize = text->searchChildren(findInitVarWithName("textSize"));
    textStyleTextColor = text->searchChildren(findInitVarWithName("textColor"));
    textStyleTextAlign = text->searchChildren(findInitVarWithName("textAlign"));
    buttonStyleTextSize = button->searchChildren(findInitVarWithName("textSize"));
    buttonStyleTextColor = button->searchChildren(findInitVarWithName("textColor"));
    buttonStyleTextAlign = button->searchChildren(findInitVarWithName("textAlign"));
    iconStyleFillColor = icon->searchChildren(findInitVarWithName("fillColor"));
    linkAttributeLink = link->searchChildren(findInitVarWithName("link"));
    arrayFunAdd = array->searchChildren(findFunctionWithName("add"));
    arrayFunEmpty = array->searchChildren(findFunctionWithName("empty"));
    arrayFunLength = array->searchChildren(findFunctionWithName("length"));

    print = root->searchChildren(findFunctionWithName("print"));
}
