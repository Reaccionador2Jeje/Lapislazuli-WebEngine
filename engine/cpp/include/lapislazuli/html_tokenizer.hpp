#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace lapislazuli::html {
enum class TokenType { Text, StartTag, EndTag, Comment };

struct Token {
    TokenType type;
    std::string name;
    std::string data;
};

class Tokenizer {
public:
    [[nodiscard]] std::vector<Token> tokenize(std::string_view input) const;
};
} // namespace lapislazuli::html
