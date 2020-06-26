#include <target/js/context.h>

#include <nodes/type.h>
#include <nodes/function.h>
#include <nodes/reference.h>
#include <nodes/expression.h>

std::string JsContext::genReference(ReferenceNode *node, Node *scope) {
    Node *current = node->selectFrom(node->dereferenceThis(scope));

    std::string params;

    if (node->hasCall) {
        if (current->type != Node::Type::Function)
            throw CompileError(current, "Expected call on method.");

        FunctionNode *e = current->as<FunctionNode>();
        std::vector<Node *> passed = node->getParameters();
        Parameters parameters = e->parameters();

        MapResult map = parameters.map(passed, node->names);

        if (!map.matches)
            throw CompileError(current, "Could not match constructor to text.");

        std::vector<std::string> result(parameters.size());

        for (size_t a = 0; a < map.map.size(); a++) {
            result[map.map[a]] = genExpression(passed[a]->as<ExpressionNode>());
        }

        for (std::string &a : result) {
            if (a.empty()) {
                a = "null";
            }
        }

        // this is probably not going to work
//        if (e->init)
//            params = fmt::format("().{}({})", reserveName(e), fmt::join(result, ", "));
//        else
        params = fmt::format("({})", fmt::join(result, ", "));
    }

    if (node->hasContent)
        throw CompileError(node, "Unimplemented reference node content.");

    std::string first;

    if (!scope && current->parent->type == Node::Type::Type) {
        if (!(current->type == Node::Type::Variable && current->as<VariableNode>()->shared))
            first = "this.";

        if (current->type == Node::Type::Function && current->as<FunctionNode>()->init)
            first = fmt::format("new {}().", reserveName(current->parent)); // . for $build later
    }

//    first += node->content;
    first += reserveName(current);

    std::string last;

    if (node->next())
        last = genReference(node->next(), node->findType(current));

    return first + params + last;
}
