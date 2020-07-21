#include <target/js/context.h>

#include <nodes/enum.h>
#include <nodes/enumname.h>

#include <sstream>

//#define ENUM_VALUE_AS_NAME

std::string JsContext::genEnum(EnumNode *node) {
    std::stringstream stream;

    for (const auto &child : node->children) {
#ifdef ENUM_VALUE_AS_NAME
        std::string value = "\'" + child->as<EnumnameNode>()->name + "\'";
#else
        size_t value = nextId();
#endif

        stream << fmt::format("\nlet {} = {}", reserveName(child.get()), value);
    }

    return stream.str();
}
