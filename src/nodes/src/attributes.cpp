#include <nodes/attributes.h>

std::unordered_map<std::string, bool> parseAttributes(Parser &parser, const std::vector<std::string> &names) {
    std::unordered_map<std::string, bool> result;

    while (!parser.empty()) {
        std::string next = parser.peek();

        if (std::find(names.begin(), names.end(), next) == names.end())
             break;

        result[next] = true;
        parser.next();
    }

    return result;
}
