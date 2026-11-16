#include "lapislazuli/browser/engine.hpp"
#include "lapislazuli/dom_parser.hpp"

#include <cassert>
#include <iostream>

int main() {
    lapislazuli::html::DocumentParser parser;
    auto document = parser.parse(
        "<p id='message'>before</p>"
        "<button id='action' onclick=\"setElementText('message', 'clicked')\">Run</button>"
        "<script>"
        "setElementText('message', 'after script');"
        "setElementAttribute('message', 'data-state', 'ready');"
        "let scriptValue = 4 + 5;"
        "</script>"
        "<script>clickElement('action');</script>"
        "<script>callMissingFunction();</script>"
        "<script src='external.js'>setElementText('message', 'must not run');</script>"
        "<script type='module'>setElementText('message', 'module must not run');</script>");

    lapislazuli::browser::Engine engine;
    const auto report = engine.executeInlineScripts(*document);

    assert(report.inlineScriptsFound == 3);
    assert(report.scriptsExecuted == 2);
    assert(report.errors.size() == 1);
    const auto* message = document->getElementById("message");
    assert(message != nullptr);
    assert(message->textContent() == "clicked");
    assert(message->getAttribute("data-state") != nullptr);
    assert(*message->getAttribute("data-state") == "ready");

    std::cout << "Document script integration tests passed.\n";
}
