#include "lapislazuli/dom_parser.hpp"
#include "lapislazuli/html_tokenizer.hpp"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

namespace lapislazuli::html {
namespace {

bool isVoidTag(std::string_view name) {
    static constexpr std::string_view tags[] = {
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr"
    };
    return std::find(std::begin(tags), std::end(tags), name) != std::end(tags);
}

bool closesParagraph(std::string_view name) {
    static constexpr std::string_view tags[] = {
        "address", "article", "aside", "blockquote", "div", "dl", "fieldset",
        "footer", "form", "h1", "h2", "h3", "h4", "h5", "h6", "header",
        "hr", "main", "nav", "ol", "p", "pre", "section", "table", "ul"
    };
    return std::find(std::begin(tags), std::end(tags), name) != std::end(tags);
}

void appendUtf8(std::string& output, unsigned long codepoint) {
    if (codepoint == 0 || codepoint > 0x10FFFF ||
        (codepoint >= 0xD800 && codepoint <= 0xDFFF)) codepoint = 0xFFFD;
    if (codepoint <= 0x7F) output.push_back(static_cast<char>(codepoint));
    else if (codepoint <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        output.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
}

std::string decodeEntities(std::string_view input) {
    std::string output;
    output.reserve(input.size());
    for (std::size_t i = 0; i < input.size();) {
        if (input[i] != '&') {
            output.push_back(input[i++]);
            continue;
        }
        const auto semicolon = input.find(';', i + 1);
        if (semicolon == std::string_view::npos || semicolon - i > 12) {
            output.push_back(input[i++]);
            continue;
        }
        const auto entity = input.substr(i + 1, semicolon - i - 1);
        if (entity == "amp") output.push_back('&');
        else if (entity == "lt") output.push_back('<');
        else if (entity == "gt") output.push_back('>');
        else if (entity == "quot") output.push_back('"');
        else if (entity == "apos") output.push_back('\'');
        else if (!entity.empty() && entity.front() == '#') {
            std::string digits(entity.substr(1));
            int base = 10;
            if (!digits.empty() && (digits.front() == 'x' || digits.front() == 'X')) {
                base = 16;
                digits.erase(digits.begin());
            }
            if (digits.empty()) {
                output.append(input.substr(i, semicolon - i + 1));
            } else {
                char* end = nullptr;
                const auto value = std::strtoul(digits.c_str(), &end, base);
                if (end == digits.c_str() || *end != '\\0')
                    output.append(input.substr(i, semicolon - i + 1));
                else appendUtf8(output, value);
            }
        } else {
            output.append(input.substr(i, semicolon - i + 1));
        }
        i = semicolon + 1;
    }
    return output;
}

bool isWhitespaceOnly(std::string_view text) {
    return std::all_of(text.begin(), text.end(), [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
    });
}

void closeNearest(std::vector<dom::Node*>& stack, std::string_view name) {
    for (std::size_t i = stack.size(); i > 1; --i) {
        if (stack[i - 1]->name() == name) {
            stack.resize(i - 1);
            return;
        }
    }
}

void closeOpenParagraph(std::vector<dom::Node*>& stack) {
    closeNearest(stack, "p");
}

} // namespace

std::unique_ptr<dom::Document> DocumentParser::parse(std::string_view source) const {
    Tokenizer tokenizer;
    const auto tokens = tokenizer.tokenize(source);
    auto document = std::make_unique<dom::Document>();
    std::vector<dom::Node*> openElements{document.get()};

    for (const Token& token : tokens) {
        switch (token.type) {
            case TokenType::Text: {
                if (token.data.empty()) break;
                if (openElements.back() == document.get() && isWhitespaceOnly(token.data)) break;
                openElements.back()->appendChild(document->createTextNode(decodeEntities(token.data)));
                break;
            }
            case TokenType::Comment:
                openElements.back()->appendChild(document->createComment(token.data));
                break;
            case TokenType::StartTag: {
                if (token.name == "html" || token.name == "head" || token.name == "body") {
                    // Explicit structural tags are retained; implicit document synthesis
                    // is handled in a later tree-construction milestone.
                }
                if (token.name == "li") closeNearest(openElements, "li");
                if (token.name == "p" || closesParagraph(token.name)) closeOpenParagraph(openElements);

                auto element = document->createElement(token.name);
                for (const auto& [name, value] : token.attributes) {
                    element->setAttribute(name, decodeEntities(value));
                }

                const std::string tagName = element->name();
                dom::Node& inserted = openElements.back()->appendChild(std::move(element));
                if (!isVoidTag(tagName) && !token.self_closing) openElements.push_back(&inserted);
                break;
            }
            case TokenType::EndTag:
                if (token.name == "p") {
                    closeNearest(openElements, "p");
                    break;
                }
                closeNearest(openElements, token.name);
                break;
        }
    }

    return document;
}

} // namespace lapislazuli::html
