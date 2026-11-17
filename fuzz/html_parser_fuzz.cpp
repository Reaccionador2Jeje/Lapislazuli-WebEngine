#include "lapislazuli/dom_parser.hpp"
#include "lapislazuli/html_tokenizer.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string_view input(reinterpret_cast<const char*>(data), size);
    (void)lapislazuli::html::Tokenizer{}.tokenize(input);
    (void)lapislazuli::html::DocumentParser{}.parse(input);
    return 0;
}
