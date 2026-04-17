#pragma once
#include <string>
#include <vector>

namespace smartdoc {

struct ParsedBlock {
    enum Type { H1, H2, H3, Paragraph, CodeBlock, List, Quote };
    Type type;
    std::string content;
};

class MarkdownParser {
public:
    MarkdownParser();
    std::vector<ParsedBlock> parse(const std::string& markdown);
    std::string toHtml(const std::string& markdown);

private:
    ParsedBlock::Type detectLineType(const std::string& line);
};

} // namespace smartdoc
