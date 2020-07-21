#pragma once

#include <nodes/root.h>

#include <cli/watcher.h>

#include <memory>
#include <thread>
#include <filesystem>

namespace fs = std::filesystem;

class Interpreter {
    const static std::string usage;

    fs::path parent;

    std::unique_ptr<RootNode> root;
    std::string output;

    std::unique_ptr<Watcher> watcher;
    std::unique_ptr<std::thread> watcherThread;

    std::vector<std::string> arguments;

    std::string loadString(const std::string &path);
    std::vector<uint8_t> loadBinary(const std::string &path);

    std::vector<uint8_t> errorMessage(const std::string &major, const std::string &details);

    void compile(const std::string &source, bool silent = false);
    void compilePath(const std::string &path, bool silent = false);

    void debug();
    void build();
    void serve();
public:
    volatile bool stop = false;

    void exec();

    Interpreter(const char **args, size_t count);
};
