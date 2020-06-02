#include <nodes/root.h>

#include <tests/list.h>

#include <fmt/printf.h>

#include <target/js/root.h>

int main(int count, const char **args) {
    if (count != 2) {
        fmt::print("Usage: page <path/to/source.page>\n");
        return -1;
    }

    try {
        RootNode root = RootNode::fromFile(args[1]);
        root.add(RootNode::fromFile("standard.page"));
        NodeList list(&root);
//        fmt::print("{}\n", list.toString());
        root.verify();
        fmt::print("{}\n", JsRoot(&root).build());
    } catch (const ParseError &e) {
        fmt::print("{}\n", e.what());
    } catch (const VerifyError &e) {
        fmt::print("{}\n", e.what());
    } catch (const CompileError &e) {
        fmt::print("{}\n", e.what());
    }

    return 0;
}
