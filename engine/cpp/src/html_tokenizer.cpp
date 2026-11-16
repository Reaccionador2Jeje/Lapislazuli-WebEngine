#include "lapislazuli/html_tokenizer.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace lapislazuli::html {
namespace {

bool isSpace(char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

bool isNameChar(char c) {
    const auto value = static_cast<unsigned char>(c);
    return std::isalnum(value) != 0 || c == '-' || c == '_' || c == ':' || value >= 128;
}

std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

void skipSpace(std::string_view input, std::size_t& position) {
    while (position < input.size() && isSpace(input[position])) ++position;
}

} // namespace

std::vector<Token> Tokenizer::tokenize(std::string_view input) const {
    std::vector<Token> tokens;
    std::size_t cursor = 0;

    while (cursor < input.size()) {
        if (input.substr(cursor, 4) == "<!--") {
            const auto end = input.find("-->", cursor + 4);
            const auto contentEnd = end == std::string_view::npos ? input.size() : end;
            tokens.push_back({TokenType::Comment, {},
                std::string(input.substr(cursor + 4, contentEnd - cursor - 4))});
            cursor = end == std::string_view::npos ? input.size() : end + 3;
            continue;
        }

        if (input[cursor] != '<') {
            const auto end = input.find('<', cursor);
            const auto textEnd = end == std::string_view::npos ? input.size() : end;
            tokens.push_back({TokenType::Text, {},
                std::string(input.substr(cursor, textEnd - cursor))});
            cursor = textEnd;
            continue;
        }

        std::size_t position = cursor + 1;
        if (position >= input.size()) {
            tokens.push_back({TokenType::Text, {}, "<"});
            break;
        }

        // Declarations and processing instructions are not DOM elements.
        if (input[position] == '!' || input[position] == '?') {
            char quote = 0;
            while (position < input.size()) {
                const char c = input[position++];
                if (quote != 0) {
                    if (c == quote) quote = 0;
                } else if (c == '"' || c == '\'') {
                    quote = c;
                } else if (c == '>') {
                    break;
                }
            }
            cursor = position;
            continue;
        }

        const bool closing = input[position] == '/';
        if (closing) ++position;
        skipSpace(input, position);

        const std::size_t nameStart = position;
        while (position < input.size() && isNameChar(input[position])) ++position;
        if (nameStart == position) {
            tokens.push_back({TokenType::Text, {}, "<"});
            ++cursor;
            continue;
        }

        Token token{closing ? TokenType::EndTag : TokenType::StartTag,
                    lowerAscii(std::string(input.substr(nameStart, position - nameStart))), {}};

        if (closing) {
            while (position < input.size() && input[position] != '>') ++position;
            if (position < input.size()) ++position;
            tokens.push_back(std::move(token));
            cursor = position;
            continue;
        }

        while (position < input.size()) {
            skipSpace(input, position);
            if (position >= input.size()) break;
            if (input[position] == '>') {
                ++position;
                break;
            }
            if (input[position] == '/' && position + 1 < input.size() &&
                input[position + 1] == '>') {
                token.self_closing = true;
                position += 2;
                break;
            }

            const std::size_t attributeStart = position;
            while (position < input.size() && !isSpace(input[position]) &&
                   input[position] != '=' && input[position] != '>' &&
                   input[position] != '/') {
                ++position;
            }
            if (attributeStart == position) {
                ++position;
                continue;
            }

            std::string attributeName = lowerAscii(
                std::string(input.substr(attributeStart, position - attributeStart)));
            skipSpace(input, position);
            std::string attributeValue;
            if (position < input.size() && input[position] == '=') {
                ++position;
                skipSpace(input, position);
                if (position < input.size() &&
                    (input[position] == '"' || input[position] == '\'')) {
                    const char quote = input[position++];
                    const auto valueStart = position;
                    while (position < input.size() && input[position] != quote) ++position;
                    attributeValue = std::string(input.substr(valueStart, position - valueStart));
                    if (position < input.size()) ++position;
                } else {
                    const auto valueStart = position;
                    while (position < input.size() && !isSpace(input[position]) &&
                           input[position] != '>') ++position;
                    attributeValue = std::string(input.substr(valueStart, position - valueStart));
                }
            }

            // HTML keeps the first duplicate attribute on a start tag.
            token.attributes.try_emplace(std::move(attributeName), std::move(attributeValue));
        }

        tokens.push_back(std::move(token));
        cursor = position;
    }

    return tokens;
}

} // namespace lapislazuli::html
