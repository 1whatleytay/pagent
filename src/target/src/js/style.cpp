#include <target/js/context.h>

#include <nodes/function.h>
#include <nodes/expression.h>

std::unordered_map<std::string, std::string> styleFormat = {
    { "color", "background-color: ${{ {} }};" },
    { "align", "margin: ${{ $alignStyle({}) }};" },
    { "width", "width: ${{ {} * 2 }}px;" },
    { "height", "height: ${{ {} * 2 }}px;" },
    { "column", "width: ${{ Math.round({}) * 100 }}%;" },
    { "row", "height: ${{ Math.round({}) * 100 }}%;" },
    { "padding", "padding: ${{ {} * 2 }}px;" },
    { "margin", "margin: ${{ {} * 2 }}px;" },
};

std::string JsContext::genStyle(
    const Parameters &params, const std::vector<ssize_t> &output, const std::vector<Node *> &values) {
    std::stringstream stream;

    std::unordered_map<Node *, std::string> referenceStyles = {
        { textStyleTextSize, "font-size: ${{ {} }}px;" },
        { buttonStyleTextSize, "font-size: ${{ {} }}px;" },
        { textStyleTextColor, "color: ${{ {} }};" },
        { buttonStyleTextColor, "color: ${{ {} }};" },
        { textStyleTextAlign, "text-align: ${{ {} }};" },
        { buttonStyleTextAlign, "text-align: ${{ {} }};" },
    };

    for (size_t a = 0; a < output.size(); a++) {
        Parameter param = params[output[a]];

        std::string expression = genExpression(values[a]->as<ExpressionNode>());

        if (param.reference) {
            auto match = referenceStyles.find(param.reference);

            if (match != referenceStyles.end())
                stream << fmt::format(match->second, expression);
        } else {
            auto match = styleFormat.find(param.name);

            if (match != styleFormat.end())
                stream << fmt::format(match->second, expression);
        }
    }

    return stream.str();
}
