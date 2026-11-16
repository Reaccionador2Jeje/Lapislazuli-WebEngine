#pragma once

#include "lapislazuli/dom.hpp"

#include <memory>
#include <string_view>

namespace lapislazuli::html {

class DocumentParser {
public:
    // Parses the supported HTML subset into a mutable DOM. This is not yet
    // the complete WHATWG tree-construction algorithm.
    [[nodiscard]] std::unique_ptr<dom::Document> parse(std::string_view source) const;
};

} // namespace lapislazuli::html
