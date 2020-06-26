#include <nodes/context.h>

#include <nodes/enum.h>
#include <nodes/type.h>
#include <nodes/string.h>
#include <nodes/function.h>
#include <nodes/variable.h>

#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void RootNode::add(const RootNode &node) {
    for (const auto &child : node.children) {
        child->parent = this;
        children.push_back(child);
    }
}

RootNode::RootNode(Parser &parser, Node *parent, const std::string &path) : Node(parent, Type::Root), path(path) {
    while (!parser.empty()) {
        std::string next = parser.peek();

        if (next == "}") {
            if (!parent)
                throw ParseError(parser, "Unexpected }} in global context.");

            break;
        } else if (next == "enum") {
            children.push_back(std::make_shared<EnumNode>(parser, this));
        } else if (next == "type" || next == "page") {
            children.push_back(std::make_shared<TypeNode>(parser, this));
        } else if (next == "var") {
            parser.next(); // var
            children.push_back(std::make_shared<VariableNode>(parser, this));
        } else if (next == "fun") {
            parser.next(); // fun
            children.push_back(std::make_shared<FunctionNode>(parser, this, false, false));
        } else if (next == "import") {
            parser.next(); // import

            StringNode location(parser, this);

            if (!location.indices.empty())
                throw ParseError(parser, "Literals must be constant, no expressions are allowed.");

            std::string importText = location.text;
            std::string importPath = fs::path(path).remove_filename() / importText;

            std::ifstream stream(importPath, std::ios::ate);
            std::vector<char> data(stream.tellg());
            stream.seekg(0, std::ios::beg);
            stream.read(data.data(), data.size());

            Parser importParser(std::string(data.begin(), data.end()));
            add(RootNode(importParser, nullptr, importPath));
        } else {
            throw ParseError(parser, "Unknown keyword {}.", next);
        }
    }
}
