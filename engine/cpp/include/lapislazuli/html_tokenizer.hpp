#pragma once
#include <string>
#include <string_view>
#include <vector>
namespace lapislazuli::html {
enum class TokenType { Text, StartTag, EndTag, Comment };
struct Attribute { std::string name; std::string value; };
struct Token { TokenType type; std::string name; std::string data; std::vector<Attribute> attributes; bool self_closing=false; };
class Tokenizer { public: [[nodiscard]] std::vector<Token> tokenize(std::string_view input) const; };
}
