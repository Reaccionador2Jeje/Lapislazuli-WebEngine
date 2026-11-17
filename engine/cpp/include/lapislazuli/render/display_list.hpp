#pragma once

#include "lapislazuli/layout/layout_engine.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace lapislazuli::render {

struct Rect {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

struct Color {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
    std::uint8_t alpha = 255;
};

enum class CommandType { FillRect, StrokeRect, DrawText, DrawImage };

// Backend-neutral drawing instruction. Native embedders can translate these
// commands to their chosen GPU, windowing, or software raster backend.
struct PaintCommand {
    CommandType type = CommandType::FillRect;
    Rect bounds;
    Color color;
    double strokeWidth = 0.0;
    double fontSize = 16.0;
    bool bold = false;
    bool italic = false;
    std::string text;
    std::string resourceUrl;
};

struct DisplayList {
    double documentWidth = 0.0;
    double documentHeight = 0.0;
    std::vector<PaintCommand> commands;
};

class DisplayListBuilder {
public:
    [[nodiscard]] DisplayList build(
        const layout::LayoutResult& layout,
        const std::unordered_map<const dom::Element*, css::ComputedStyle>& styles) const;
};

} // namespace lapislazuli::render
