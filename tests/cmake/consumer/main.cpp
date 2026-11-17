#include <lapislazuli/lapislazuli.hpp>

#include <filesystem>
#include <iostream>
#include <string>

int main() {
    using namespace lapislazuli;

    const auto version = core::EngineRuntime::engineVersion();
    const auto modules = core::EngineRuntime::modules();
    if (modules.size() != 10 ||
        core::EngineRuntime::findModule("security") == nullptr ||
        core::EngineRuntime::findModule("render") == nullptr) {
        std::cerr << "SDK module registry is incomplete\n";
        return 1;
    }

    const auto database = std::filesystem::temp_directory_path() /
        "lapislazuli-downstream-consumer.sqlite";
    std::error_code ignored;
    std::filesystem::remove(database, ignored);
    std::filesystem::remove(database.string() + "-wal", ignored);
    std::filesystem::remove(database.string() + "-shm", ignored);

    {
        browser::BrowserConfig config;
        config.storageDatabasePath = database.string();
        config.viewport = {800, 600, 16};
        browser::Browser browser(config);
        auto page = browser.loadHtml(
            "<html><head><style>#card { display:block; width:120px; "
            "height:40px; background-color:#224466; }</style></head>"
            "<body><div id='card'>SDK ready</div>"
            "<script>setElementAttribute('card','data-sdk','ok');</script>"
            "</body></html>",
            "https://consumer.example.test/");

        const auto* card = page.document().getElementById("card");
        if (!card || !card->getAttribute("data-sdk") ||
            *card->getAttribute("data-sdk") != "ok" ||
            page.origin().serialize() != "https://consumer.example.test" ||
            page.storage() == nullptr || page.displayList().commands.empty()) {
            std::cerr << "Installed Browser/Page SDK smoke test failed\n";
            return 2;
        }
        page.storage()->setItem("consumer-check", "passed");
    }

    std::filesystem::remove(database, ignored);
    std::filesystem::remove(database.string() + "-wal", ignored);
    std::filesystem::remove(database.string() + "-shm", ignored);

    std::cout << core::EngineRuntime::engineName()
              << " " << version.major << "." << version.minor << "." << version.patch
              << " (" << modules.size() << " modules; Browser/Page SDK passed)\n";
    return 0;
}
