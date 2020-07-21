#include <cli/cli.h>

#include <cli/watcher.h>

#include <tests/list.h>

#include <target/error.h>
#include <target/js/context.h>

#include <http/http.h>
#include <sockets/sockets.h>

#include <fmt/format.h>

#include <thread>
#include <fstream>
#include <iostream>

const std::string Interpreter::usage = "Usage: page <server/build/debug> path/to/source.page";

std::string Interpreter::loadString(const std::string &path) {
    std::ifstream stream(path, std::ios::ate);

    if (!stream.is_open())
        throw std::runtime_error(fmt::format("Could not read binary file {}.", path));

    std::vector<uint8_t> data(stream.tellg());
    stream.seekg(0, std::ios::beg);

    stream.read(reinterpret_cast<char *>(data.data()), data.size());

    return std::string(data.begin(), data.end());
}

std::vector<uint8_t> Interpreter::loadBinary(const std::string &path) {
    std::ifstream stream(path, std::ios::ate | std::ios::binary);

    if (!stream.is_open())
        throw std::runtime_error(fmt::format("Could not read binary file {}.", path));

    std::vector<uint8_t> data(stream.tellg());
    stream.seekg(0, std::ios::beg);

    stream.read(reinterpret_cast<char *>(data.data()), data.size());

    return data;
}

std::string replace(std::string text, std::string find, std::string replace) {
    // i cant be bothered to https://stackoverflow.com/a/3418285
    if (text.empty())
        return "";

    size_t start_pos = 0;
    while((start_pos = text.find(find, start_pos)) != std::string::npos) {
        text.replace(start_pos, find.size(), replace);
        start_pos += replace.size();
    }

    return text;
}

std::vector<uint8_t> Interpreter::errorMessage(const std::string &major, const std::string &details) {
    std::string templateHTML = loadString(parent / "error.html");
    std::string result = replace(replace(templateHTML, "{{ MAJOR_PROBLEM }}", major), "{{ MINOR_PROBLEM }}", details);

    return std::vector<uint8_t>(result.begin(), result.end());
}

void Interpreter::compile(const std::string &source, bool silent) {
    std::string standardPath = parent / "standard.page";

    if (!silent)
        fmt::print("Building standard....\n");
    Parser standardParser(loadString(standardPath));
    RootNode standard(standardParser, nullptr, standardPath);

    if (!silent)
        fmt::print("Building root...\n");
    Parser rootParser(source);
    root = std::make_unique<RootNode>(rootParser, nullptr, "");
    root->add(standard);
    root->verify();

    if (!silent)
        fmt::print("Generating output...\n");
    output = JsContext(root.get()).build();

    if (!silent)
        fmt::print("OK\n");
}

void Interpreter::compilePath(const std::string &path, bool silent) {
    compile(loadString(path), silent);
}

void Interpreter::debug() {
    if (arguments.size() < 2)
        throw std::runtime_error(usage);

    if (arguments[1] == "stdin") {
        size_t length = 0;
        std::cin >> length;

        std::vector<char> data(length);
        std::cin.read(data.data(), data.size());

        compile(std::string(data.begin(), data.end()), true);

        fmt::print("{}\n", output);
    } else {
        if (arguments.size() < 3)
            throw std::runtime_error(usage);

        compilePath(arguments[2], true);

        if (arguments[1] == "nodes") {
            fmt::print("{}\n", NodeList(root.get()).toString());
        }

        if (arguments[1] == "js") {
            fmt::print("{}\n", output);
        }
    }
}

void Interpreter::build() {
    if (arguments.size() < 2)
        throw std::runtime_error(usage);

    compilePath(arguments[1]);

    std::string destination = "app.js";

    if (arguments.size() > 2)
        destination = arguments[2];

    std::ofstream stream(destination, std::ios::trunc);
    stream.write(output.c_str(), output.size());
}

void Interpreter::serve() {
    if (arguments.size() < 2)
        throw std::runtime_error(usage);

    std::string sourcePath = arguments[1];
    std::string watchPath = sourcePath;

    if (arguments.size() >= 3)
        watchPath = arguments[2];

    volatile bool needsRecompile = true;

    watcher = std::make_unique<Watcher>(watchPath, [&needsRecompile](std::string path) {
        needsRecompile = true;
    });

    watcherThread = std::make_unique<std::thread>([this]() { watcher->exec(); });

    sockets::ServerSocket server(80);

    fmt::print("Serving on http://localhost:{}\n", server.getPort());

    std::vector<uint8_t> outputData;
    std::vector<uint8_t> css = loadBinary(parent / "framework.css");
    std::vector<uint8_t> framework = loadBinary(parent / "framework.js");
    std::vector<uint8_t> index = loadBinary(parent / "index.html");
    std::vector<uint8_t> error;

    while (!stop) {
        if (needsRecompile) {
            try {
                compilePath(sourcePath);

                outputData = std::vector<uint8_t>(output.begin(), output.end());
                error.clear();
            } catch (const ParseError &e) { // haha
                fmt::print("{}\n", e.what());
                error = errorMessage("ParseError", e.what());
            } catch (const VerifyError &e) {
                fmt::print("{}\n", e.what());
                error = errorMessage("VerifyError", e.what());
            } catch (const CompileError &e) {
                fmt::print("{}\n", e.what());
                error = errorMessage("CompileError", e.what());
            } catch (const std::runtime_error &e) {
                fmt::print("{}\n", e.what());
                error = errorMessage("GenericError", e.what());
            }

            needsRecompile = false;
        }

        if (server.ready()) {
            sockets::ConnectionSocket conn(server);

            http::ReadCallback read = [&conn](void *data, size_t size) -> size_t {
                return conn.read(data, size);
            };

            http::WriteCallback write = [&conn](const void *data, size_t size) -> size_t {
                return conn.write(data, size);
            };

            auto request = http::Request().receive(read);

            if (request.path == "/framework.js") {
                http::Response().content(framework).send(write);
            } else if (request.path == "/app.js") {
                http::Response().content(outputData).send(write);
            } else if (request.path == "/framework.css") {
                http::Response().content(css).send(write);
            } else {
                http::Response().content(error.empty() ? index : error).send(write);
            }
        }
    }
}

void Interpreter::exec() {
    try {
        if (arguments.size() < 1)
            throw std::runtime_error(usage);

        if (arguments[0] == "debug")
            debug();

        if (arguments[0] == "build")
            build();

        if (arguments[0] == "serve")
            serve();
    } catch (const ParseError &e) { // haha
        fmt::print("ParseError: {}\n", e.what());
    } catch (const VerifyError &e) {
        fmt::print("VerifyError: {}\n", e.what());
    } catch (const CompileError &e) {
        fmt::print("CompileError: {}\n", e.what());
    } catch (const std::runtime_error &e) {
        fmt::print("{}\n", e.what());
    }

    if (watcher) {
        watcher->stop = true;
        watcherThread->join();
    }
}

Interpreter::Interpreter(const char **args, size_t count) {
    parent = fs::path(args[0]).parent_path();

    arguments.resize(count - 1);
    for (size_t a = 1; a < count; a++) {
        arguments[a - 1] = args[a];
    }
}
