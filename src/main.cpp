#include <cli/cli.h>

int main(int count, const char **args) {
    Interpreter(args, count).exec();

//    if (count != 3) {
//        fmt::print("Usage: page <serve|build|debug> ...\n");
//        return -1;
//    }
//
//    try {
//        RootNode root = RootNode::fromFile(args[2]);
//        root.add(RootNode::fromFile("standard.page"));
//        NodeList list(&root);
//        root.verify();
//        std::string output = JsRoot(&root).build();
//
//        if (strcmp(args[1], "debug") == 0) {
//            fmt::print("{}\n", output);
//        } else if (strcmp(args[1], "serve") == 0) {
//            sockets::ServerSocket server(80);
//
//            std::vector<uint8_t> outputData(output.begin(), output.end());
//            std::vector<uint8_t> framework = loadFile("framework.js");
//            std::vector<uint8_t> index = loadFile("index.html");
//
//            while (true) {
//                sockets::ConnectionSocket conn(server);
//
//                http::ReadCallback read = [&conn](void *data, size_t size) -> size_t {
//                    return conn.read(data, size);
//                };
//
//                http::WriteCallback write = [&conn](const void *data, size_t size) -> size_t {
//                    return conn.write(data, size);
//                };
//
//                auto request = http::Request().receive(read);
//
//                if (request.path == "/framework.js") {
//                    http::Response().content(framework).send(write);
//                } else if (request.path == "/app.js") {
//                    http::Response().content(outputData).send(write);
//                } else {
//                    http::Response().content(index).send(write);
//                }
//            }
//        } else {
//            fmt::format("Usage: page <serve|build|debug> ...\n");
//        }
//    } catch (const ParseError &e) {
//        fmt::print("{}\n", e.what());
//    } catch (const VerifyError &e) {
//        fmt::print("{}\n", e.what());
//    } catch (const CompileError &e) {
//        fmt::print("{}\n", e.what());
//    }

    return 0;
}
