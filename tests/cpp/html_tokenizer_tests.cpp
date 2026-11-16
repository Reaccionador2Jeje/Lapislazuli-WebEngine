#include "lapislazuli/html_tokenizer.hpp"
#include <cassert>
#include <iostream>

int main() {
    const lapislazuli::html::Tokenizer tokenizer;
    const auto tokens = tokenizer.tokenize("<html>Hello</html><!--note-->");
    assert(tokens.size() == 4);
    assert(tokens[0].type == lapislazuli::html::TokenType::StartTag);
    assert(tokens[0].name == "html");
    assert(tokens[1].type == lapislazuli::html::TokenType::Text);
    assert(tokens[1].data == "Hello");
    assert(tokens[2].type == lapislazuli::html::TokenType::EndTag);
    assert(tokens[3].type == lapislazuli::html::TokenType::Comment);
    assert(tokens[3].data == "note");
    std::cout << "HTML tokenizer tests passed.\n";
}
