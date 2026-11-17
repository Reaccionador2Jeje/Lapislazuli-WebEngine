#include "lapislazuli/browser/engine.hpp"
#include "lapislazuli/dom_parser.hpp"

#include <cassert>
#include <iostream>

int main() {
    using lapislazuli::browser::Engine;

    const std::string base = "https://example.com/a/b/page.html?old=1";
    assert(Engine::resolveUrl(base, "../css/site.css") ==
           "https://example.com/a/css/site.css");
    assert(Engine::resolveUrl(base, "/images/logo.png") ==
           "https://example.com/images/logo.png");
    assert(Engine::resolveUrl(base, "//cdn.example.net/app.js") ==
           "https://cdn.example.net/app.js");
    assert(Engine::resolveUrl(base, "?v=2") ==
           "https://example.com/a/b/page.html?v=2");
    assert(Engine::resolveUrl(base, "#section") ==
           "https://example.com/a/b/page.html?old=1#section");
    assert(Engine::resolveUrl(base, "") ==
           "https://example.com/a/b/page.html?old=1");
    assert(Engine::resolveUrl(base, "https://static.example.net/x.css") ==
           "https://static.example.net/x.css");

    bool rejected = false;
    try {
        (void)Engine::resolveUrl(base, "javascript:alert(1)");
    } catch (const lapislazuli::browser::NavigationError&) {
        rejected = true;
    }
    assert(rejected);
    std::string nulReference = "image.png";
    nulReference.push_back(char(0));
    nulReference += "hidden";
    rejected = false;
    try { (void)Engine::resolveUrl(base, nulReference); }
    catch (const lapislazuli::browser::NavigationError&) { rejected = true; }
    assert(rejected);

    lapislazuli::html::DocumentParser parser;
    auto document = parser.parse(
        "<style>.item { color: red; }</style>"
        "<script>let comparison = 1 < 2;</script>"
        "<img src='../images/logo.png'>");
    const auto styles = document->getElementsByTagName("style");
    const auto scripts = document->getElementsByTagName("script");
    const auto images = document->getElementsByTagName("img");
    assert(styles.size() == 1 && styles[0]->textContent() == ".item { color: red; }");
    assert(scripts.size() == 1 && scripts[0]->textContent() == "let comparison = 1 < 2;");
    assert(images.size() == 1 && *images[0]->getAttribute("src") == "../images/logo.png");

    std::cout << "Resource URL and document resource parsing tests passed.\n";
}
