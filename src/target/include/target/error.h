#pragma once

#include <nodes/node.h>

#include <tests/list.h>

#include <fmt/format.h>

#include <exception>

class CompileError : std::exception {
public:
    std::string issue;

    const char* what() const noexcept override {
        return issue.c_str();
    }

    template <typename ... A>
    CompileError(Node *node, const char *message, A ... args) {
        std::string messageResult = fmt::format(message, args ...);

        std::string nodeText = NodeList(node).toString();

        if (nodeText.size() > 100)
            nodeText = nodeText.substr(0, 100) + "...";

        issue = fmt::format("{}\n{}", messageResult, nodeText);
    }
};
