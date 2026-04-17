#include "parser.h"
#include <sstream>
#include <algorithm>

namespace smartdoc {

MarkdownParser::MarkdownParser() {
}

std::vector<ParsedBlock> MarkdownParser::parse(const std::string& markdown) {
    std::vector<ParsedBlock> blocks;
    std::istringstream stream(markdown);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        ParsedBlock block;
        block.type = detectLineType(line);
        block.content = line;
        blocks.push_back(block);
    }

    return blocks;
}

std::string MarkdownParser::toHtml(const std::string& markdown) {
    // 简单实现，实际应用应该更完善
    std::string html = "<html><body>";
    auto blocks = parse(markdown);

    for (const auto& block : blocks) {
        switch (block.type) {
            case ParsedBlock::H1:
                html += "<h1>" + block.content + "</h1>";
                break;
            case ParsedBlock::H2:
                html += "<h2>" + block.content + "</h2>";
                break;
            case ParsedBlock::Paragraph:
                html += "<p>" + block.content + "</p>";
                break;
            default:
                html += "<p>" + block.content + "</p>";
        }
    }

    html += "</body></html>";
    return html;
}

ParsedBlock::Type MarkdownParser::detectLineType(const std::string& line) {
    if (line.find("# ") == 0) return ParsedBlock::H1;
    if (line.find("## ") == 0) return ParsedBlock::H2;
    if (line.find("### ") == 0) return ParsedBlock::H3;
    if (line.find("```") == 0) return ParsedBlock::CodeBlock;
    if (line.find("- ") == 0 || line.find("* ") == 0) return ParsedBlock::List;
    if (line.find("> ") == 0) return ParsedBlock::Quote;
    return ParsedBlock::Paragraph;
}

} // namespace smartdoc
