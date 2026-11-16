#pragma once

#include "lapislazuli/dom.hpp"
#include <memory>
#include <string_view>

namespace lapislazuli::html {

class DocumentParser {
public:
    // Builds a small DOM tree from the current experimental tokenizer.
    // Attribute parsing and full HTML error-recovery rules are not implemented yet.
    [[nodiscard]] std::unique_ptr<dom::Document> parse(std::string_view source) const;
};

} // namespace lapislazuli::html
