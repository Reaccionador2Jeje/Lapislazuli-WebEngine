#include "lapislazuli/html_tokenizer.hpp"
#include <cctype>
#include <string>
#include <utility>

namespace lapislazuli::html {
namespace {
bool is_space(char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}
std::string trim(std::string value) {
    while (!value.empty() && is_space(value.front())) value.erase(value.begin());
    while (!value.empty() && is_space(value.back())) value.pop_back();
    return value;
}
} // namespace

std::vector<Token> Tokenizer::tokenize(std::string_view input) const {
    std::vector<Token> tokens;
    std::size_t cursor = 0;
    while (cursor < input.size()) {
        if (input.substr(cursor, 4) == "<!--") {
            const auto end = input.find("-->", cursor + 4);
            const auto content_end = end == std::string_view::npos ? input.size() : end;
            tokens.push_back({TokenType::Comment, {},
                std::string(input.substr(cursor + 4, content_end - cursor - 4))});
            cursor = end == std::string_view::npos ? input.size() : end + 3;
            continue;
        }
        if (input[cursor] == '<') {
            const auto end = input.find('>', cursor + 1);
            if (end == std::string_view::npos) {
                tokens.push_back({TokenType::Text, {}, std::string(input.substr(cursor))});
                break;
            }
            std::string inside = trim(std::string(input.substr(cursor + 1, end - cursor - 1)));
            const bool closing = !inside.empty() && inside.front() == '/';
            if (closing) inside.erase(inside.begin());
            const auto name_end = inside.find_first_of(" \t\r\n/");
            const std::string name = inside.substr(0, name_end);
            if (!name.empty()) {
                tokens.push_back({closing ? TokenType::EndTag : TokenType::StartTag, name, {}});
            } else {
                tokens.push_back({TokenType::Text, {}, std::string(input.substr(cursor, end - cursor + 1))});
            }
            cursor = end + 1;
            continue;
        }
        const auto end = input.find('<', cursor);
        const auto text_end = end == std::string_view::npos ? input.size() : end;
        tokens.push_back({TokenType::Text, {}, std::string(input.substr(cursor, text_end - cursor))});
        cursor = text_end;
    }
    return tokens;
}
} // namespace lapislazuli::html
