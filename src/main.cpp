#include <nodes/root.h>

#include <tests/list.h>

#include <fmt/printf.h>

#include <vector>
#include <fstream>

int main(int count, const char **args) {
    if (count != 2) {
        fmt::print("Usage: page <path/to/source.page>\n");
        return -1;
    }

    std::ifstream stream(args[1], std::ios::ate);
    std::vector<char> data(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(data.data(), data.size());
    stream.close();

    Parser parser(std::string(data.begin(), data.end()));

    try {
        RootNode root(parser, nullptr);
        NodeList list(&root);
        fmt::print("{}\n", list.toString());
//        root.verify();
    } catch (const ParseError &e) {
        fmt::print("{}\n", e.what());
    } catch (const VerifyError &e) {
        fmt::print("{}\n", e.what());
    }

    return 0;
}
