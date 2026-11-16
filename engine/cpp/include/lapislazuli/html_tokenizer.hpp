#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace lapislazuli::html {

enum class TokenType { Text, StartTag, EndTag, Comment };

struct Token {
    TokenType type;
    std::string name;
    std::string data;
    std::unordered_map<std::string, std::string> attributes = {};
    bool self_closing = false;
};

class Tokenizer {
public:
    // A practical tokenizer for the currently supported HTML subset.
    // It preserves quoted attribute values and treats malformed '<' as text.
    [[nodiscard]] std::vector<Token> tokenize(std::string_view input) const;
};

} // namespace lapislazuli::html
