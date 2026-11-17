#include "lapislazuli/css/css_engine.hpp"
#include "lapislazuli/dom_parser.hpp"
#include "lapislazuli/layout/layout_engine.hpp"
#include "lapislazuli/render/display_list.hpp"

#include <cassert>
#include <iostream>
#include <vector>

int main() {
    using namespace lapislazuli;

    html::DocumentParser parser;
    auto document = parser.parse(
        "<html><body>"
        "<div class='panel'><p>Hello rendering</p>"
        "<img src='https://example.com/logo.png'></div>"
        "</body></html>");

    const auto sheet = css::Parser{}.parse(
        ".panel { display:block; width:200px; height:100px; padding:10px; "
        "background-color:#336699; color:#ffffff; border-top:2px; "
        "border-color:#000000; font-size:20px; font-weight:bold; }"
        "p { display:block; }"
        "img { width:32px; height:24px; }");

    const std::vector<const css::StyleSheet*> sheets{&sheet};
    const auto styles = css::StyleResolver{}.computeTree(*document, sheets);
    const auto layout = layout::LayoutEngine{}.layout(*document, styles, {400, 300, 16});
    const auto display = render::DisplayListBuilder{}.build(layout, styles);

    assert(display.documentWidth == 400);
    assert(display.documentHeight >= 300);
    assert(display.commands.size() == 4);

    assert(display.commands[0].type == render::CommandType::FillRect);
    assert(display.commands[0].color.red == 0x33);
    assert(display.commands[0].color.green == 0x66);
    assert(display.commands[0].color.blue == 0x99);

    assert(display.commands[1].type == render::CommandType::StrokeRect);
    assert(display.commands[1].strokeWidth == 2);

    assert(display.commands[2].type == render::CommandType::DrawText);
    assert(display.commands[2].text == "Hello rendering");
    assert(display.commands[2].fontSize == 20);
    assert(display.commands[2].bold);
    assert(display.commands[2].color.red == 255);

    assert(display.commands[3].type == render::CommandType::DrawImage);
    assert(display.commands[3].resourceUrl == "https://example.com/logo.png");
    assert(display.commands[3].bounds.width == 32);
    assert(display.commands[3].bounds.height == 24);

    std::cout << "Display-list rendering tests passed.\n";
}
