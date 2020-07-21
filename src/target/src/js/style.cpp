#include <target/js/context.h>

#include <nodes/function.h>
#include <nodes/expression.h>

std::unordered_map<std::string, std::string> styleFormat = {
    { "color", "background-color: #${{ {}.hex }};" },
    { "width", "width: ${{ {} * 2 }}px;" },
    { "height", "height: ${{ {} * 2 }}px;" },
    { "column", "width: ${{ Math.round({} * 100) }}%;" },
    { "row", "height: ${{ Math.round({} * 100) }}%;" },
    { "padding", "padding: ${{ $directionStyle({}) }};" },
    { "margin", "margin: ${{ $directionStyle({}) }};" },
    { "border", "${{ $borderStyle({}) }}" },
    { "rounded", "border-radius: ${{ {} }}px;" },
    { "stretch", "flex: 1;" }
};

std::unordered_map<std::string, std::string> styleEnd = {
    // semicolons added by framework
    { "align", "${{ $alignStyle({}) }}" },
    { "contentAlign", "${{ $contentAlignStyle({}) }}" },
};

std::string JsContext::genStyle(
    const Parameters &params, const std::vector<ssize_t> &output, const std::vector<Node *> &values) {
    std::stringstream stream;

    std::unordered_map<Node *, std::string> referenceStyles = {
        { textStyleTextSize, "font-size: ${{ {} }}px;" },
        { buttonStyleTextSize, "font-size: ${{ {} }}px;" },
        { textStyleTextColor, "color: #${{ {}.hex }};" },
        { buttonStyleTextColor, "color: #${{ {}.hex }};" },
        { textStyleTextAlign, "text-align: ${{ $textAlignStyle({}) }};" },
        { buttonStyleTextAlign, "text-align: ${{ $textAlignStyle({}) }};" },
        { iconStyleFillColor, "color: #${{ {}.hex }}; fill: currentColor;" },
    };

    std::stringstream end;

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
            else {
                auto endMatch = styleEnd.find(param.name);

                if (endMatch != styleEnd.end())
                    end << fmt::format(endMatch->second, expression);
            }
        }
    }

    return stream.str() + end.str();
}
