#include <nodes/comment.h>

CommentNode::CommentNode(Parser &parser, Node *parent) : Node(parent, Type::Comment) {
    // simple comments until lexer is here :D
    size_t point = parser.select();

    if (parser.next() != "/" || parser.next() != "/") {
        parser.jump(point);

        if (parser.next() != "/" || parser.next() != "*") {
            parser.jump(point);

            throw ParseError(parser, "Internal comment error, expected comment.");
        } else { // matches /*
           content = parser.until({ "*/" }); // doesn't match * /

            parser.next(); // *
            parser.next(); // /
        }
    } else { // matches //
        content = parser.until({ "\n" });
    }
}
