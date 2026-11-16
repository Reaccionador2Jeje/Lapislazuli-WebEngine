#pragma once
#include "lapislazuli/dom.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
namespace lapislazuli::css {
enum class TokenType { Ident, Hash, String, Whitespace, Dot, Colon, Comma, Greater, OpenBrace, CloseBrace, Semicolon, Bang, Delim, End };
struct Token { TokenType type; std::string text; std::size_t offset=0; };
class Tokenizer { public: [[nodiscard]] std::vector<Token> tokenize(std::string_view source) const; };
struct SimpleSelector { std::string tag; std::string id; std::vector<std::string> classes; };
enum class Combinator { Descendant, Child };
struct Selector { std::vector<SimpleSelector> parts; std::vector<Combinator> combinators; [[nodiscard]] bool matches(const dom::Element&) const; [[nodiscard]] unsigned specificity() const noexcept; };
struct Declaration { std::string property; std::string value; bool important=false; };
struct Rule { std::vector<Selector> selectors; std::vector<Declaration> declarations; std::size_t source_order=0; };
class StyleSheet { public: [[nodiscard]] const std::vector<Rule>& rules() const noexcept { return rules_; } private: friend class Parser; std::vector<Rule> rules_; };
class Parser { public: [[nodiscard]] StyleSheet parse(std::string_view source) const; };
class ComputedStyle { public: [[nodiscard]] const std::string* get(std::string_view property) const noexcept; [[nodiscard]] bool contains(std::string_view property) const noexcept; [[nodiscard]] const std::unordered_map<std::string,std::string>& properties() const noexcept { return properties_; } private: friend class StyleResolver; std::unordered_map<std::string,std::string> properties_; };
class StyleResolver {
public:
 [[nodiscard]] ComputedStyle compute(const dom::Element&,const std::vector<const StyleSheet*>&) const;
 [[nodiscard]] std::unordered_map<const dom::Element*,ComputedStyle> computeTree(const dom::Document&,const std::vector<const StyleSheet*>&) const;
};
}
