#pragma once

#include <nodes/context.h>

#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

class Interpreter {
    const static std::string usage;

    fs::path parent;

    std::unique_ptr<RootNode> root;
    std::string output;

    std::vector<std::string> arguments;

    std::string loadString(const std::string &path);
    std::vector<uint8_t> loadBinary(const std::string &path);

    void compile(const std::string &path);

    void debug();
    void build();
    void serve();
public:
    volatile bool stop = false;

    void exec();

    Interpreter(const char **args, size_t count);
};
