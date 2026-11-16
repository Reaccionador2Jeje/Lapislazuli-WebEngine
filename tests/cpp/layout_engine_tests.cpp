#include "lapislazuli/css/css_engine.hpp"
#include "lapislazuli/dom.hpp"
#include "lapislazuli/layout/layout_engine.hpp"

#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

int main() {
    using namespace lapislazuli;
    dom::Document document;
    auto html = document.createElement("html");
    auto body = document.createElement("body");
    auto card = document.createElement("div");
    card->setAttribute("class", "card");
    auto inner = document.createElement("section");
    inner->setAttribute("class", "inner");
    inner->appendChild(document.createTextNode("Layout works"));
    card->appendChild(std::move(inner));
    body->appendChild(std::move(card));
    html->appendChild(std::move(body));
    document.appendChild(std::move(html));

    auto sheet = css::Parser{}.parse(
        ".card { width:200px; height:50px; padding:10px; margin:5px; }"
        ".inner { width:50%; height:20px; }"
        ".hidden { display:none; }");
    std::vector<const css::StyleSheet*> sheets{&sheet};
    const auto styles = css::StyleResolver{}.computeTree(document, sheets);

    const auto result = layout::LayoutEngine{}.layout(document, styles, {800, 600, 16});
    assert(result.document_width == 800);
    assert(result.document_height >= 600);
    assert(result.root.display == "block");
    assert(result.root.children.size() == 1);

    const auto& body_box = result.root.children.front();
    assert(body_box.display == "block");
    assert(body_box.children.size() == 1);

    const auto& card_box = body_box.children.front();
    assert(card_box.display == "block");
    assert(card_box.width == 200);
    assert(card_box.height == 50);
    assert(card_box.margin_top == 5);
    assert(card_box.padding_left == 10);
    assert(card_box.children.size() == 1);

    const auto& inner_box = card_box.children.front();
    assert(inner_box.width == 100);
    assert(inner_box.height == 20);
    assert(inner_box.children.size() == 1);
    assert(inner_box.children.front().text == "Layout works");

    std::cout << "Layout engine tests passed.\n";
}
