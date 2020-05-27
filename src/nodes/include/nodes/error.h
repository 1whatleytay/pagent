#pragma once

#include <langutil/parser.h>

#include <fmt/format.h>

#include <exception>

class ParseError : std::exception {
public:
    std::string issue;

    const char* what() const noexcept override {
        return issue.c_str();
    }

    template <typename ... A>
    ParseError(langutil::Parser &parser, const char *message, A ... args) {
//        parser.rollback(); // undo what caused the issue hopefully...

        size_t index = parser.select();
        std::string text = parser.content();

        uint32_t lineNumber = 0;
        for (size_t a = 0; a < index; a++) {
            if (text[a] == '\n')
                lineNumber++;
        }

        size_t startOfLine = text.find_last_of('\n', index) + 1;
        if (startOfLine == std::string::npos)
            startOfLine = 0;
        size_t endOfLine = text.find_first_of('\n', startOfLine);
        std::string line = text.substr(startOfLine, endOfLine - startOfLine);

        std::string messageResult = fmt::format(message, args ...);

        issue = fmt::format("{} line {}\n -> {}", messageResult, lineNumber + 1, line);
    }
};

class VerifyError : std::exception {
public:
    std::string issue;

    const char* what() const noexcept override {
        return issue.c_str();
    }

    template <typename ... A>
    VerifyError(const char *message, A ... args) {
        std::string messageResult = fmt::format(message, args ...);

        issue = fmt::format("{}", messageResult);
    }
};
