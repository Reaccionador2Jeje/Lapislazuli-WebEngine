#include "lapislazuli/html_tokenizer.hpp"
#include <iostream>
#include <string>

namespace {
const char* token_name(lapislazuli::html::TokenType type) {
    using lapislazuli::html::TokenType;
    switch (type) {
        case TokenType::Text: return "TEXT";
        case TokenType::StartTag: return "START_TAG";
        case TokenType::EndTag: return "END_TAG";
        case TokenType::Comment: return "COMMENT";
    }
    return "UNKNOWN";
}
}

int main() {
    std::cout << "Lapislazuli WebEngine 0.1.0-dev\n";
    std::cout << "Paste HTML, then finish input (Ctrl+D / Ctrl+Z):\n";
    std::string input, line;
    while (std::getline(std::cin, line)) input += line + '\n';
    const lapislazuli::html::Tokenizer tokenizer;
    for (const auto& token : tokenizer.tokenize(input)) {
        std::cout << token_name(token.type);
        if (!token.name.empty()) std::cout << " name=\"" << token.name << "\"";
        if (!token.data.empty()) std::cout << " data=\"" << token.data << "\"";
        std::cout << '\n';
    }
}
