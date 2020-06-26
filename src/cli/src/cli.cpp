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

void Interpreter::compile(const std::string &path) {
    std::string standardPath = parent / "standard.page";

    fmt::print("Building standard....\n");
    Parser standardParser(loadString(standardPath));
    RootNode standard(standardParser, nullptr, standardPath);

    fmt::print("Building root...\n");
    Parser rootParser(loadString(path));
    root = std::make_unique<RootNode>(rootParser, nullptr, path);
    root->add(standard);
    root->verify();

    fmt::print("Generating output...\n");
    output = JsContext(root.get()).build();

    fmt::print("OK\n");
}

void Interpreter::debug() {
    if (arguments.size() < 2)
        throw std::runtime_error(usage);

    compile(arguments[1]);

    fmt::print("{}\n\n{}\n", NodeList(root.get()).toString(), output);
}

void Interpreter::build() {
    if (arguments.size() < 2)
        throw std::runtime_error(usage);

    compile(arguments[1]);

    assert(false); // not going to bother lol
}

void Interpreter::serve() {
    if (arguments.size() < 2)
        throw std::runtime_error(usage);

    std::string sourcePath = arguments[1];

    compile(sourcePath);

    volatile bool needsRecompile = false;

    Watcher watcher(sourcePath, [&needsRecompile](std::string path) {
        needsRecompile = true;
    });

    std::thread watcherThread([&watcher]() { watcher.exec(); });

    sockets::ServerSocket server(80);

    fmt::print("Serving on http://localhost:80\n");

    std::vector<uint8_t> outputData(output.begin(), output.end());
    std::vector<uint8_t> css = loadBinary(parent / "framework.css");
    std::vector<uint8_t> framework = loadBinary(parent / "framework.js");
    std::vector<uint8_t> index = loadBinary(parent / "index.html");

    while (!stop) {
        if (needsRecompile) {
            compile(sourcePath);

            outputData = std::vector<uint8_t>(output.begin(), output.end());

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
                http::Response().content(index).send(write);
            }
        }
    }

    watcher.stop = true;
    watcherThread.join();
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
        fmt::print("{}\n", e.what());
    } catch (const VerifyError &e) {
        fmt::print("{}\n", e.what());
    } catch (const CompileError &e) {
        fmt::print("{}\n", e.what());
    } catch (const std::runtime_error &e) {
        fmt::print("{}\n", e.what());
    }
}

Interpreter::Interpreter(const char **args, size_t count) {
    parent = fs::path(args[0]).parent_path();

    arguments.resize(count - 1);
    for (size_t a = 1; a < count; a++) {
        arguments[a - 1] = args[a];
    }
}
