#include <target/js/context.h>

#include <nodes/function.h>
#include <nodes/expression.h>

std::unordered_map<std::string, std::string> attributeFormat = {
    { "click", " onclick=\"$callEvent(${{ $routeEvent(() => {{ return ({})() }}) }})\"" },
    { "mouseEnter", " onmouseover=\"$callEvent(${{ $routeEvent(() => {{ return ({})() }}) }})\"" },
    { "mouseExit", " onmouseout=\"$callEvent(${{ $routeEvent(() => {{ return ({})() }}) }})\"" },
};

std::string JsContext::genAttributes(
    const Parameters &params, const std::vector<ssize_t> &output, const std::vector<Node *> &values) {
    std::stringstream stream;

    std::unordered_map<Node *, std::string> referenceStyles = {
        { linkAttributeLink, " href=\"${{ {} }}\"" },
    };

    for (size_t a = 0; a < output.size(); a++) {
        Parameter param = params[output[a]];

        std::string expression = genExpression(values[a]->as<ExpressionNode>());

        if (param.reference) {
            auto match = referenceStyles.find(param.reference);

            if (match != referenceStyles.end())
                stream << fmt::format(match->second, expression);
        } else {
            auto match = attributeFormat.find(param.name);

            if (match != attributeFormat.end())
                stream << fmt::format(match->second, expression);
        }
    }

    return stream.str();
}
