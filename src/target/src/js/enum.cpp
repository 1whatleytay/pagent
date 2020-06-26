#include <target/js/context.h>

#include <nodes/enum.h>
#include <nodes/enumname.h>

#include <sstream>

std::string JsContext::genEnum(EnumNode *node) {
    std::stringstream stream;

    for (const auto &child : node->children) {
        stream << fmt::format("\nlet {} = \'{}\'", reserveName(child.get()), child->as<EnumnameNode>()->name);
    }

    return stream.str();
}
