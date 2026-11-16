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
    const auto richer = tokenizer.tokenize(
        "<IMG SRC='images/a.png' alt=\"A & B\" disabled data-id=7 />"
        "<!doctype html><p>ok</p>");
    assert(richer.size() == 4);
    assert(richer[0].name == "img");
    assert(richer[0].self_closing);
    assert(richer[0].attributes.at("src") == "images/a.png");
    assert(richer[0].attributes.at("alt") == "A & B");
    assert(richer[0].attributes.at("disabled").empty());
    assert(richer[0].attributes.at("data-id") == "7");
    assert(richer[1].name == "p");
    std::cout << "HTML tokenizer tests passed.\n";
}
