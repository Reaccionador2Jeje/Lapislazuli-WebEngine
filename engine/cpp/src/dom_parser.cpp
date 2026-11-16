#include "lapislazuli/dom_parser.hpp"
#include "lapislazuli/html_tokenizer.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>
#include <vector>

namespace lapislazuli::html {
namespace {

std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool isVoidTag(std::string_view name) {
    static constexpr std::string_view tags[] = {
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr"
    };
    return std::find(std::begin(tags), std::end(tags), name) != std::end(tags);
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
                if (!token.data.empty()) {
                    openElements.back()->appendChild(document->createTextNode(token.data));
                }
                break;
            }
            case TokenType::Comment: {
                openElements.back()->appendChild(document->createComment(token.data));
                break;
            }
            case TokenType::StartTag: {
                if (!token.name.empty() && token.name.front() == '!') break;
                auto element = document->createElement(lowerAscii(token.name));
                const std::string tagName = element->name();
                dom::Node& inserted = openElements.back()->appendChild(std::move(element));
                if (!isVoidTag(tagName)) openElements.push_back(&inserted);
                break;
            }
            case TokenType::EndTag: {
                // Basic recovery: close the nearest matching open element.
                for (std::size_t i = openElements.size(); i > 1; --i) {
                    if (openElements[i - 1]->name() == lowerAscii(token.name)) {
                        openElements.resize(i - 1);
                        break;
                    }
                }
                break;
            }
        }
    }

    return document;
}

} // namespace lapislazuli::html
