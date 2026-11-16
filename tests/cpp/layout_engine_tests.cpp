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

    auto flex = document.createElement("div");
    flex->setAttribute("class", "row");
    auto first = document.createElement("div");
    first->setAttribute("class", "item");
    first->appendChild(document.createTextNode("One"));
    auto second = document.createElement("div");
    second->setAttribute("class", "item");
    second->appendChild(document.createTextNode("Two"));
    flex->appendChild(std::move(first));
    flex->appendChild(std::move(second));
    body->appendChild(std::move(flex));
    html->appendChild(std::move(body));
    document.appendChild(std::move(html));

    auto sheet = css::Parser{}.parse(
        ".card { width:200px; height:50px; padding:10px; margin:5px; }"
        ".inner { width:50%; height:20px; }"
        ".row { display:flex; width:300px; height:100px; gap:10px; justify-content:center; align-items:center; }"
        ".item { width:50px; height:20px; }"
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
    assert(body_box.children.size() == 2);

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

    const auto& flex_box = body_box.children.back();
    assert(flex_box.display == "flex");
    assert(flex_box.width == 300);
    assert(flex_box.height == 100);
    assert(flex_box.children.size() == 2);
    assert(flex_box.children[0].width == 50);
    assert(flex_box.children[1].width == 50);
    assert(flex_box.children[1].x - flex_box.children[0].x == 60);

    std::cout << "Layout engine tests passed.\n";
}
