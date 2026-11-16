#include "lapislazuli/js/runtime.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int main() {
    using namespace lapislazuli;

    js::Runtime runtime;
    std::vector<std::string> output;
    runtime.setOutput([&output](std::string_view line) { output.emplace_back(line); });

    auto result = runtime.execute(
        "let base = 7; const factor = 3; "
        "let total = base * factor + 2; "
        "print('total:', total); "
        "let message = 'Lapislazuli ' + String(total); "
        "let valid = total >= 20 && true;");

    assert(result.toString() == "true");
    assert(runtime.globals().at("total").toNumber() == 23.0);
    assert(runtime.globals().at("message").toString() == "Lapislazuli 23");
    assert(runtime.globals().at("valid").toBoolean());
    assert(output.size() == 1);
    assert(output.front() == "total: 23");

    dom::Document document;
    auto html = document.createElement("html");
    auto paragraph = document.createElement("p");
    paragraph->setAttribute("id", "welcome");
    paragraph->appendChild(document.createTextNode("Hello from DOM"));
    html->appendChild(std::move(paragraph));
    document.appendChild(std::move(html));

    runtime.bindDocument(document);
    assert(runtime.execute("getElementText('welcome')").toString() == "Hello from DOM");
    assert(runtime.execute("getElementText('missing')").toString() == "null");

    bool rejected = false;
    try {
        (void)runtime.execute("unknownValue + 1");
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    assert(rejected);

    std::cout << "JavaScript runtime tests passed.\n";
}
