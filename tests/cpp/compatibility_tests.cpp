#include "lapislazuli/browser/engine.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    using namespace lapislazuli;

    const std::string html =
        "<html><head><title>Compatibility fixture</title>"
        "<style>"
        "#panel { display:block; width:220px; height:80px; padding:8px; "
        "background-color:#123456; color:#ffffff; font-size:18px; "
        "font-weight:bold; border-top:2px; border-color:#000000; }"
        "p { display:block; }"
        "</style></head><body>"
        "<div id='panel'><p id='message'>Before</p></div>"
        "<script>"
        "setElementText('message', 'Compatibility works');"
        "setElementAttribute('message', 'data-ready', 'yes');"
        "</script>"
        "</body></html>";

    browser::Engine engine;
    const auto result = engine.loadHtml(html, "https://compat.example.test/index.html",
                                        {640, 480, 16});

    assert(result.statusCode == 200);
    assert(result.origin.serialize() == "https://compat.example.test");
    assert(result.document != nullptr);
    assert(result.scripts.inlineScriptsFound == 1);
    assert(result.scripts.scriptsExecuted == 1);
    assert(result.scripts.errors.empty());
    assert(result.styleSheets.size() == 1);

    const auto* message = result.document->getElementById("message");
    assert(message != nullptr);
    assert(message->textContent() == "Compatibility works");
    assert(message->getAttribute("data-ready") != nullptr);
    assert(*message->getAttribute("data-ready") == "yes");

    const auto* panel = result.document->getElementById("panel");
    assert(panel != nullptr);
    const auto style = result.computedStyles.find(panel);
    assert(style != result.computedStyles.end());
    assert(style->second.get("background-color") != nullptr);
    assert(*style->second.get("background-color") == "#123456");

    assert(result.layoutTree.root.element != nullptr);
    assert(result.layoutTree.document_width == 640);
    assert(result.displayList.documentWidth == 640);
    assert(result.displayList.commands.size() == 3);
    assert(result.displayList.commands[0].type == render::CommandType::FillRect);
    assert(result.displayList.commands[1].type == render::CommandType::StrokeRect);
    assert(result.displayList.commands[2].type == render::CommandType::DrawText);
    assert(result.displayList.commands[2].text == "Compatibility works");
    assert(result.displayList.commands[2].fontSize == 18);
    assert(result.displayList.commands[2].bold);

    std::cout << "End-to-end HTML/CSS/JS/layout/render compatibility tests passed.\n";
}
