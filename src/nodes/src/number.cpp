#include <nodes/number.h>

static bool isDigits(const std::string &text) {
    for (char c : text) {
        if (!std::isdigit(c))
            return false;
    }

    return true;
}

NumberNode::NumberNode(Parser &parser, Node *parent) : Node(parent, Type::Number) {
    std::string number = parser.next();

    // 0x and 0b?

    if (!isDigits(number))
        throw ParseError(parser, "Expected number but got {}.", number);

    if (parser.peek() == ".") {
        parser.next(); // .

        std::string next = parser.next();
        if (!isDigits(next))
            throw ParseError(parser, "Expected number lower but got {}.", next);

        number += "." + next;
    }

    value = std::stod(number);
}
