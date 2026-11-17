#include "lapislazuli/browser/browser.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

using lapislazuli::browser::Browser;
using lapislazuli::browser::BrowserConfig;
using lapislazuli::browser::BrowserError;
using lapislazuli::security::Origin;

template <typename Function>
void rejects(Function&& function) {
    bool caught = false;
    try { function(); } catch (const BrowserError&) { caught = true; }
    assert(caught);
}

int main() {
    const auto path = std::filesystem::temp_directory_path() /
        "lapislazuli-browser-sdk-tests.sqlite";
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    std::filesystem::remove(path.string() + "-wal", ignored);
    std::filesystem::remove(path.string() + "-shm", ignored);

    BrowserConfig config;
    config.storageDatabasePath = path.string();
    config.viewport = {1024, 768, 16};
    Browser browser(config);

    auto page = browser.loadHtml(
        "<html><body><p id='welcome'>Hello SDK</p></body></html>",
        "https://Example.com/welcome");
    assert(page.origin().serialize() == "https://example.com");
    assert(page.document().getElementById("welcome") != nullptr);
    assert(page.storage() != nullptr);
    page.storage()->setItem("page-key", "page-value");

    auto first = browser.openStorage(Origin::fromUrl("HTTPS://Example.COM:443/one"));
    first->setItem("session-theme", "dark");

    auto sameOrigin = browser.openStorage(Origin::fromUrl("https://example.com/two"));
    assert(sameOrigin->getItem("session-theme") ==
           std::optional<std::string>("dark"));
    assert(sameOrigin->getItem("page-key") ==
           std::optional<std::string>("page-value"));

    auto otherOrigin = browser.openStorage(Origin::fromUrl("https://other.example/"));
    assert(!otherOrigin->getItem("session-theme").has_value());

    Browser withoutStorage;
    rejects([&] {
        (void)withoutStorage.openStorage(Origin::fromUrl("https://example.com/"));
    });
    rejects([&] {
        (void)browser.openStorage(Origin{});
    });

    std::filesystem::remove(path, ignored);
    std::filesystem::remove(path.string() + "-wal", ignored);
    std::filesystem::remove(path.string() + "-shm", ignored);
    std::cout << "Browser SDK profile and origin storage tests passed.\n";
}
