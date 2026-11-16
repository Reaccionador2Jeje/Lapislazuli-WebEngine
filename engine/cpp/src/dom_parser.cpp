#include "lapislazuli/dom_parser.hpp"
#include "lapislazuli/html_tokenizer.hpp"

#include <algorithm>
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
                openElements.back()->appendChild(document->createTextNode(token.data));
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
                    element->setAttribute(name, value);
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
