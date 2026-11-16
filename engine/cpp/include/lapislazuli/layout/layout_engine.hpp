#pragma once

#include "lapislazuli/css/css_engine.hpp"
#include "lapislazuli/dom.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace lapislazuli::layout {

struct Viewport {
    double width = 1280.0;
    double height = 720.0;
    double root_font_size = 16.0;
};

struct LayoutBox {
    const dom::Element* element = nullptr;
    std::string text;
    std::string display;
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
    double margin_top = 0.0;
    double margin_right = 0.0;
    double margin_bottom = 0.0;
    double margin_left = 0.0;
    double padding_top = 0.0;
    double padding_right = 0.0;
    double padding_bottom = 0.0;
    double padding_left = 0.0;
    double border_top = 0.0;
    double border_right = 0.0;
    double border_bottom = 0.0;
    double border_left = 0.0;
    std::vector<LayoutBox> children;
};

struct LayoutResult {
    LayoutBox root;
    double document_width = 0.0;
    double document_height = 0.0;
};

class LayoutEngine {
public:
    [[nodiscard]] LayoutResult layout(
        const dom::Document& document,
        const std::unordered_map<const dom::Element*, css::ComputedStyle>& styles,
        Viewport viewport = {}) const;
};

} // namespace lapislazuli::layout
