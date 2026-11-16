#pragma once
#include "lapislazuli/dom.hpp"
#include <memory>
#include <string_view>
namespace lapislazuli::html {
class DocumentParser {
public:
 // Experimental parser; does not implement the complete WHATWG HTML algorithm.
 [[nodiscard]] std::unique_ptr<dom::Document> parse(std::string_view source) const;
};
}
