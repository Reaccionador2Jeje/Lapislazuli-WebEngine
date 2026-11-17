#include "lapislazuli/layout/layout_engine.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <string_view>

namespace lapislazuli::layout {
namespace {

bool isBlockTag(std::string_view tag) {
    static constexpr std::string_view tags[] = {
        "html", "body", "address", "article", "aside", "blockquote", "div",
        "dl", "fieldset", "figcaption", "figure", "footer", "form",
        "h1", "h2", "h3", "h4", "h5", "h6", "header", "hr",
        "main", "nav", "ol", "p", "pre", "section", "table", "ul", "li"
    };
    return std::find(std::begin(tags), std::end(tags), tag) != std::end(tags);
}

bool isNonRenderedTag(std::string_view tag) {
    static constexpr std::string_view tags[] = {
        "head", "style", "script", "meta", "link", "title", "base", "template"
    };
    return std::find(std::begin(tags), std::end(tags), tag) != std::end(tags);
}

double number(std::string_view value, double fallback) {
    std::string copy(value);
    char* end = nullptr;
    const double parsed = std::strtod(copy.c_str(), &end);
    return end == copy.c_str() || !std::isfinite(parsed) ? fallback : parsed;
}

double resolveLength(const css::Length& length, double basis, double font_size,
                     double root_font_size, double fallback) {
    switch (length.unit) {
        case css::LengthUnit::Number:
        case css::LengthUnit::Px: return length.value;
        case css::LengthUnit::Percent: return basis * length.value / 100.0;
        case css::LengthUnit::Em: return font_size * length.value;
        case css::LengthUnit::Rem: return root_font_size * length.value;
        case css::LengthUnit::Auto: return fallback;
    }
    return fallback;
}

double propertyLength(const css::ComputedStyle* style, std::string_view name,
                      double basis, double font_size, double root_font_size,
                      double fallback = 0.0) {
    if (!style) return fallback;
    const auto value = style->length(name);
    return value ? resolveLength(*value, basis, font_size, root_font_size, fallback)
                 : fallback;
}

double lineHeight(const css::ComputedStyle* style, double font_size) {
    if (!style) return font_size * 1.2;
    const auto* value = style->get("line-height");
    if (!value || *value == "normal") return font_size * 1.2;
    const double parsed = number(*value, font_size * 1.2);
    if (parsed > 0.0) return parsed < 4.0 ? parsed * font_size : parsed;
    return font_size * 1.2;
}

void translate(LayoutBox& box, double dx, double dy) {
    box.x += dx;
    box.y += dy;
    for (auto& child : box.children) translate(child, dx, dy);
}

struct Builder {
    const std::unordered_map<const dom::Element*, css::ComputedStyle>& styles;
    double root_font_size;

    const css::ComputedStyle* styleFor(const dom::Element& element) const {
        const auto found = styles.find(&element);
        return found == styles.end() ? nullptr : &found->second;
    }

    LayoutBox build(const dom::Element& element, double x, double y,
                    double containing_width, double containing_height,
                    bool has_height_basis, double parent_font_size) const {
        LayoutBox box;
        box.element = &element;
        const auto* style = styleFor(element);
        box.display = isNonRenderedTag(element.name()) ? "none" :
            (style && style->get("display")
                ? *style->get("display")
                : (isBlockTag(element.name()) ? "block" : "inline"));
        if (box.display == "none") return box;

        const double font_size = std::max(1.0,
            propertyLength(style, "font-size", containing_width, parent_font_size,
                           root_font_size, parent_font_size));
        const double line_height = lineHeight(style, font_size);

        box.margin_top = propertyLength(style, "margin-top", containing_width, font_size, root_font_size);
        box.margin_right = propertyLength(style, "margin-right", containing_width, font_size, root_font_size);
        box.margin_bottom = propertyLength(style, "margin-bottom", containing_width, font_size, root_font_size);
        box.margin_left = propertyLength(style, "margin-left", containing_width, font_size, root_font_size);
        box.padding_top = std::max(0.0, propertyLength(style, "padding-top", containing_width, font_size, root_font_size));
        box.padding_right = std::max(0.0, propertyLength(style, "padding-right", containing_width, font_size, root_font_size));
        box.padding_bottom = std::max(0.0, propertyLength(style, "padding-bottom", containing_width, font_size, root_font_size));
        box.padding_left = std::max(0.0, propertyLength(style, "padding-left", containing_width, font_size, root_font_size));
        box.border_top = std::max(0.0, propertyLength(style, "border-top", containing_width, font_size, root_font_size));
        box.border_right = std::max(0.0, propertyLength(style, "border-right", containing_width, font_size, root_font_size));
        box.border_bottom = std::max(0.0, propertyLength(style, "border-bottom", containing_width, font_size, root_font_size));
        box.border_left = std::max(0.0, propertyLength(style, "border-left", containing_width, font_size, root_font_size));

        const double horizontal_edges = box.margin_left + box.margin_right +
            box.padding_left + box.padding_right + box.border_left + box.border_right;
        const auto width_value = style ? style->length("width") : std::nullopt;
        const bool width_auto = !width_value || width_value->unit == css::LengthUnit::Auto;
        double specified_width = width_auto ? containing_width - horizontal_edges
            : resolveLength(*width_value, containing_width, font_size, root_font_size, 0.0);
        specified_width = std::max(0.0, specified_width);
        if (style && style->get("box-sizing") && *style->get("box-sizing") == "border-box") {
            specified_width = std::max(0.0, specified_width - box.padding_left -
                box.padding_right - box.border_left - box.border_right);
        }
        box.width = std::max(0.0, specified_width);
        box.x = x + box.margin_left + box.border_left + box.padding_left;
        box.y = y + box.margin_top + box.border_top + box.padding_top;

        const auto height_value = style ? style->length("height") : std::nullopt;
        const bool height_auto = !height_value || height_value->unit == css::LengthUnit::Auto;
        double cursor_y = box.y;
        double inline_width = 0.0;
        double inline_height = 0.0;

        for (const auto& child : element.children()) {
            if (child->type() == dom::NodeType::Text) {
                const auto& text = child->data();
                if (text.empty()) continue;
                LayoutBox text_box;
                text_box.text = text;
                text_box.display = "inline";
                text_box.width = std::min(box.width, static_cast<double>(text.size()) * font_size * 0.5);
                text_box.height = line_height;
                text_box.x = box.x + inline_width;
                text_box.y = cursor_y;
                if (inline_width > 0.0 && inline_width + text_box.width > box.width) {
                    cursor_y += std::max(line_height, inline_height);
                    inline_width = 0.0;
                    inline_height = 0.0;
                    text_box.x = box.x;
                    text_box.y = cursor_y;
                }
                inline_width += text_box.width;
                inline_height = std::max(inline_height, text_box.height);
                box.children.push_back(std::move(text_box));
                continue;
            }
            if (child->type() != dom::NodeType::Element) continue;

            const auto& child_element = static_cast<const dom::Element&>(*child);
            auto child_box = build(child_element, box.x, cursor_y,
                                   box.width, 0.0, false, font_size);
            if (child_box.display == "none") continue;

            if (child_box.display == "inline" && child_box.width >= box.width) {
                const auto* child_style = styleFor(child_element);
                const double child_font = std::max(1.0,
                    propertyLength(child_style, "font-size", box.width, font_size,
                                   root_font_size, font_size));
                const double estimated_text = static_cast<double>(
                    child_element.textContent().size()) * child_font * 0.5;
                const auto explicit_width = child_style ? child_style->length("width") : std::nullopt;
                if (!explicit_width || explicit_width->unit == css::LengthUnit::Auto)
                    child_box.width = std::min(box.width, estimated_text +
                        child_box.padding_left + child_box.padding_right +
                        child_box.border_left + child_box.border_right);
                child_box.height = std::max(child_box.height, lineHeight(child_style, child_font));
            }

            if (child_box.display == "block" || child_box.display == "list-item" ||
                child_box.display == "table" || child_box.display == "flex" ||
                child_box.display == "grid") {
                if (inline_width > 0.0 || inline_height > 0.0) {
                    cursor_y += std::max(line_height, inline_height);
                    inline_width = 0.0;
                    inline_height = 0.0;
                }
                child_box = build(child_element, box.x, cursor_y,
                                  box.width, 0.0, false, font_size);
                cursor_y += child_box.margin_top + child_box.border_top +
                    child_box.padding_top + child_box.height +
                    child_box.padding_bottom + child_box.border_bottom +
                    child_box.margin_bottom;
            } else {
                child_box.x = box.x + inline_width;
                child_box.y = cursor_y;
                if (inline_width > 0.0 && inline_width + child_box.width > box.width) {
                    cursor_y += std::max(line_height, inline_height);
                    inline_width = 0.0;
                    inline_height = 0.0;
                    child_box.x = box.x;
                    child_box.y = cursor_y;
                }
                inline_width += child_box.width + child_box.margin_left +
                    child_box.margin_right;
                inline_height = std::max(inline_height, child_box.height);
            }
            box.children.push_back(std::move(child_box));
        }

        if (inline_width > 0.0 || inline_height > 0.0)
            cursor_y += std::max(line_height, inline_height);
        double content_height = std::max(0.0, cursor_y - box.y);
        if (!height_auto && height_value->unit != css::LengthUnit::Percent) {
            content_height = std::max(0.0, resolveLength(*height_value,
                containing_height, font_size, root_font_size, content_height));
            if (style && style->get("box-sizing") && *style->get("box-sizing") == "border-box")
                content_height = std::max(0.0, content_height - box.padding_top -
                    box.padding_bottom - box.border_top - box.border_bottom);
        } else if (!height_auto && has_height_basis) {
            content_height = std::max(0.0, resolveLength(*height_value,
                containing_height, font_size, root_font_size, content_height));
        }
        if (box.display == "flex") {
            const bool row_direction = !style || !style->get("flex-direction") ||
                *style->get("flex-direction") == "row" ||
                *style->get("flex-direction") == "row-reverse";
            const bool reverse = style && style->get("flex-direction") &&
                (*style->get("flex-direction") == "row-reverse" ||
                 *style->get("flex-direction") == "column-reverse");
            const double gap = std::max(0.0,
                propertyLength(style, "gap", containing_width, font_size, root_font_size));
            const std::string justify = style && style->get("justify-content")
                ? *style->get("justify-content") : "flex-start";
            const std::string align = style && style->get("align-items")
                ? *style->get("align-items") : "stretch";

            if (row_direction) {
                double used = gap * (box.children.empty() ? 0.0 :
                    static_cast<double>(box.children.size() - 1));
                double cross_size = 0.0;
                for (auto& child : box.children) {
                    if (child.element) {
                        const auto* child_style = styleFor(*child.element);
                        const auto explicit_width = child_style
                            ? child_style->length("width") : std::nullopt;
                        if (!explicit_width || explicit_width->unit == css::LengthUnit::Auto) {
                            const double child_font = std::max(1.0,
                                propertyLength(child_style, "font-size", box.width,
                                               font_size, root_font_size, font_size));
                            const double text_width = static_cast<double>(
                                child.element->textContent().size()) * child_font * 0.5;
                            child.width = std::min(box.width, text_width +
                                child.padding_left + child.padding_right +
                                child.border_left + child.border_right);
                        }
                    }
                    used += child.width + child.margin_left + child.margin_right;
                    cross_size = std::max(cross_size,
                        child.height + child.margin_top + child.margin_bottom);
                }
                double offset = 0.0;
                double distributed_gap = gap;
                if (justify == "center") offset = std::max(0.0, (box.width - used) / 2.0);
                else if (justify == "flex-end") offset = std::max(0.0, box.width - used);
                else if (justify == "space-between" && box.children.size() > 1)
                    distributed_gap = std::max(gap,
                        (box.width - (used - gap * static_cast<double>(box.children.size() - 1))) /
                        static_cast<double>(box.children.size() - 1));

                double cursor_x = box.x + offset;
                for (std::size_t index = 0; index < box.children.size(); ++index) {
                    auto& child = box.children[reverse ? box.children.size() - 1 - index : index];
                    double child_y = box.y + child.margin_top;
                    if (align == "center")
                        child_y += std::max(0.0, (cross_size - child.height) / 2.0);
                    else if (align == "flex-end")
                        child_y += std::max(0.0, cross_size - child.height);
                    const double target_x = cursor_x + child.margin_left;
                    translate(child, target_x - child.x, child_y - child.y);
                    cursor_x += child.margin_left + child.width + child.margin_right +
                        distributed_gap;
                }
                content_height = std::max(content_height, cross_size);
            } else {
                double cursor = box.y;
                if (reverse) std::reverse(box.children.begin(), box.children.end());
                for (auto& child : box.children) {
                    const double target_y = cursor + child.margin_top;
                    translate(child, box.x + child.margin_left - child.x,
                              target_y - child.y);
                    cursor += child.margin_top + child.height + child.margin_bottom + gap;
                }
                if (!box.children.empty()) cursor -= gap;
                content_height = std::max(content_height, cursor - box.y);
            }
        }

        box.height = content_height;
        return box;
    }
};

} // namespace

LayoutResult LayoutEngine::layout(
    const dom::Document& document,
    const std::unordered_map<const dom::Element*, css::ComputedStyle>& styles,
    Viewport viewport) const {
    LayoutResult result;
    const auto* root = document.documentElement();
    if (!root) return result;

    const double width = std::max(0.0, viewport.width);
    const double height = std::max(0.0, viewport.height);
    Builder builder{styles, viewport.root_font_size};
    result.root = builder.build(*root, 0.0, 0.0, width, height, true, viewport.root_font_size);
    result.document_width = std::max(width, result.root.width +
        result.root.margin_left + result.root.margin_right +
        result.root.padding_left + result.root.padding_right +
        result.root.border_left + result.root.border_right);
    result.document_height = std::max(height, result.root.height +
        result.root.margin_top + result.root.margin_bottom +
        result.root.padding_top + result.root.padding_bottom +
        result.root.border_top + result.root.border_bottom);
    return result;
}

} // namespace lapislazuli::layout
