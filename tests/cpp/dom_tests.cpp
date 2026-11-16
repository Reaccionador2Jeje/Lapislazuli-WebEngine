#include "lapislazuli/dom.hpp"
#include "lapislazuli/dom_parser.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

using lapislazuli::dom::Document;
using lapislazuli::dom::Element;
using lapislazuli::dom::NodeType;

int main() {
    Document document;
    auto html = document.createElement("HTML");
    html->setAttribute("lang", "en");
    auto body = document.createElement("body");
    body->setAttribute("id", "main");
    auto paragraph = document.createElement("p");
    paragraph->appendChild(document.createTextNode("Hello <web>"));
    body->appendChild(std::move(paragraph));
    html->appendChild(std::move(body));
    Element& root = static_cast<Element&>(document.appendChild(std::move(html)));

    assert(document.documentElement() == &root);
    assert(document.getElementById("main") != nullptr);
    assert(document.getElementById("missing") == nullptr);
    assert(document.getElementsByTagName("p").size() == 1);
    assert(root.textContent() == "Hello <web>");
    assert(root.serialize() ==
        "<body id=\"main\"><p>Hello &lt;web&gt;</p></body>");

    auto* bodyNode = document.getElementById("main");
    assert(bodyNode != nullptr && bodyNode->parent() == &root);
    auto detached = root.removeChild(*bodyNode);
    assert(detached->parent() == nullptr);
    assert(document.getElementById("main") == nullptr);
    root.appendChild(std::move(detached));

    bool rejectedLeafChild = false;
    try {
        auto textNode = document.createTextNode("leaf");
        textNode->appendChild(document.createTextNode("invalid"));
    } catch (const std::invalid_argument&) {
        rejectedLeafChild = true;
    }
    assert(rejectedLeafChild);

    lapislazuli::html::DocumentParser parser;
    const auto parsed = parser.parse("<html><body><!--note--><p>Hi</p><br></body></html>");
    assert(parsed->documentElement() != nullptr);
    assert(parsed->getElementsByTagName("p").size() == 1);
    assert(parsed->textContent() == "Hi");
    assert(parsed->serialize() == "<html><body><!--note--><p>Hi</p><br></body></html>");

    const auto rich = parser.parse(
        "<!doctype html><div id='main' data-kind=primary title='A &amp; B'>"
        "<p class='one'>A &lt; B &amp; C</p><p>Second</p></div>");
    const auto* main = rich->getElementById("main");
    assert(main != nullptr);
    assert(main->getAttribute("data-kind") != nullptr);
    assert(*main->getAttribute("data-kind") == "primary");
    assert(*main->getAttribute("title") == "A & B");
    assert(rich->getElementsByTagName("p").size() == 2);
    assert(rich->textContent() == "A < B & CSecond");

    assert(parsed->getElementsByTagName("*").size() == 4);
    assert(parsed->documentElement()->type() == NodeType::Element);

    std::cout << "DOM engine tests passed.\n";
}
