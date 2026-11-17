#include "lapislazuli/render/display_list.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string_view>

namespace lapislazuli::render {
namespace {

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool parseHex(std::string_view value, Color& color) {
    auto component = [](std::string_view text, std::uint8_t& out) {
        unsigned parsed = 0;
        for (const char ch : text) {
            unsigned digit = 0;
            if (ch >= '0' && ch <= '9') digit = static_cast<unsigned>(ch - '0');
            else if (ch >= 'a' && ch <= 'f') digit = static_cast<unsigned>(ch - 'a' + 10);
            else if (ch >= 'A' && ch <= 'F') digit = static_cast<unsigned>(ch - 'A' + 10);
            else return false;
            parsed = parsed * 16 + digit;
        }
        if (parsed > 255) return false;
        out = static_cast<std::uint8_t>(parsed);
        return true;
    };

    if (value.size() == 4) {
        std::uint8_t r = 0, g = 0, b = 0;
        if (!component(value.substr(1, 1), r) ||
            !component(value.substr(2, 1), g) ||
            !component(value.substr(3, 1), b)) return false;
        color = {static_cast<std::uint8_t>(r * 17),
                 static_cast<std::uint8_t>(g * 17),
                 static_cast<std::uint8_t>(b * 17), 255};
        return true;
    }
    if (value.size() == 7 || value.size() == 9) {
        Color parsed;
        if (!component(value.substr(1, 2), parsed.red) ||
            !component(value.substr(3, 2), parsed.green) ||
            !component(value.substr(5, 2), parsed.blue)) return false;
        if (value.size() == 9 && !component(value.substr(7, 2), parsed.alpha))
            return false;
        color = parsed;
        return true;
    }
    return false;
}

bool parseColor(std::string_view raw, Color& color) {
    const auto value = lower(std::string(raw));
    if (value == "transparent") { color = {0, 0, 0, 0}; return true; }
    if (value == "black") { color = {0, 0, 0, 255}; return true; }
    if (value == "white") { color = {255, 255, 255, 255}; return true; }
    if (value == "red") { color = {255, 0, 0, 255}; return true; }
    if (value == "green") { color = {0, 128, 0, 255}; return true; }
    if (value == "blue") { color = {0, 0, 255, 255}; return true; }
    if (value == "yellow") { color = {255, 255, 0, 255}; return true; }
    if (value == "gray" || value == "grey") { color = {128, 128, 128, 255}; return true; }
    if (value == "silver") { color = {192, 192, 192, 255}; return true; }
    if (value == "purple") { color = {128, 0, 128, 255}; return true; }
    if (value == "orange") { color = {255, 165, 0, 255}; return true; }
    if (!value.empty() && value.front() == '#') return parseHex(value, color);
    return false;
}

const css::ComputedStyle* findStyle(
    const dom::Element* element,
    const std::unordered_map<const dom::Element*, css::ComputedStyle>& styles) {
    if (!element) return nullptr;
    const auto found = styles.find(element);
    return found == styles.end() ? nullptr : &found->second;
}

double fontSize(const css::ComputedStyle* style, double parentSize, double rootSize) {
    if (!style) return parentSize;
    const auto value = style->length("font-size");
    if (!value) return parentSize;
    switch (value->unit) {
        case css::LengthUnit::Number:
        case css::LengthUnit::Px: return std::max(1.0, value->value);
        case css::LengthUnit::Em: return std::max(1.0, value->value * parentSize);
        case css::LengthUnit::Rem: return std::max(1.0, value->value * rootSize);
        default: return parentSize;
    }
}

Color textColor(const css::ComputedStyle* style, Color inherited) {
    if (!style) return inherited;
    const auto* value = style->get("color");
    Color parsed;
    return value && parseColor(*value, parsed) ? parsed : inherited;
}

void appendBox(const layout::LayoutBox& box,
               const std::unordered_map<const dom::Element*, css::ComputedStyle>& styles,
               DisplayList& list, const css::ComputedStyle* inheritedStyle,
               Color inheritedColor, double inheritedFont, double rootFont) {
    const auto* style = findStyle(box.element, styles);
    const auto* activeStyle = style ? style : inheritedStyle;
    const Color color = textColor(activeStyle, inheritedColor);
    const double size = fontSize(activeStyle, inheritedFont, rootFont);
    const bool visible = !activeStyle || !activeStyle->get("visibility") ||
                         *activeStyle->get("visibility") != "hidden";

    if (box.element && visible) {
        const double left = box.x - box.padding_left - box.border_left;
        const double top = box.y - box.padding_top - box.border_top;
        const double width = box.width + box.padding_left + box.padding_right +
                             box.border_left + box.border_right;
        const double height = box.height + box.padding_top + box.padding_bottom +
                              box.border_top + box.border_bottom;

        if (style) {
            if (const auto* background = style->get("background-color")) {
                Color fill;
                if (parseColor(*background, fill) && fill.alpha != 0) {
                    PaintCommand command;
                    command.type = CommandType::FillRect;
                    command.bounds = {left, top, width, height};
                    command.color = fill;
                    list.commands.push_back(std::move(command));
                }
            }
            const double borderWidth = std::max({box.border_top, box.border_right,
                box.border_bottom, box.border_left});
            if (borderWidth > 0.0) {
                Color borderColor{0, 0, 0, 255};
                if (const auto* value = style->get("border-color")) {
                    Color parsed;
                    if (parseColor(*value, parsed)) borderColor = parsed;
                }
                PaintCommand command;
                command.type = CommandType::StrokeRect;
                command.bounds = {left, top, width, height};
                command.color = borderColor;
                command.strokeWidth = borderWidth;
                list.commands.push_back(std::move(command));
            }
        }

        if (box.element->name() == "img") {
            if (const auto* src = box.element->getAttribute("src")) {
                PaintCommand command;
                command.type = CommandType::DrawImage;
                command.bounds = {left, top, width, height};
                command.resourceUrl = *src;
                list.commands.push_back(std::move(command));
            }
        }
    }

    if (!box.text.empty() && visible) {
        PaintCommand command;
        command.type = CommandType::DrawText;
        command.bounds = {box.x, box.y, box.width, box.height};
        command.color = color;
        command.fontSize = size;
        if (activeStyle) {
            if (const auto* weight = activeStyle->get("font-weight"))
                command.bold = *weight == "bold" || *weight == "bolder" ||
                    *weight == "600" || *weight == "700" || *weight == "800" || *weight == "900";
            if (const auto* fontStyle = activeStyle->get("font-style"))
                command.italic = *fontStyle == "italic" || *fontStyle == "oblique";
        }
        command.text = box.text;
        list.commands.push_back(std::move(command));
    }

    for (const auto& child : box.children)
        appendBox(child, styles, list, activeStyle, color, size, rootFont);
}

} // namespace

DisplayList DisplayListBuilder::build(
    const layout::LayoutResult& layout,
    const std::unordered_map<const dom::Element*, css::ComputedStyle>& styles) const {
    DisplayList result;
    result.documentWidth = layout.document_width;
    result.documentHeight = layout.document_height;
    appendBox(layout.root, styles, result, nullptr, {0, 0, 0, 255}, 16.0, 16.0);
    return result;
}

} // namespace lapislazuli::render
