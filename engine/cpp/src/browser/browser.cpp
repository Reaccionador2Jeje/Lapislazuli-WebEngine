#include "lapislazuli/browser/browser.hpp"

#include <utility>

namespace lapislazuli::browser {

Page::Page(NavigationResult navigation,
           std::unique_ptr<storage::StorageArea> storage)
    : navigation_(std::move(navigation)), storage_(std::move(storage)) {}

const dom::Document& Page::document() const {
    if (!navigation_.document) throw BrowserError("Page has no document");
    return *navigation_.document;
}

dom::Document& Page::document() {
    if (!navigation_.document) throw BrowserError("Page has no document");
    return *navigation_.document;
}

Browser::Browser(BrowserConfig config)
    : config_(std::move(config)), engine_(config_.network) {}

Page Browser::navigate(NavigationRequest request) const {
    request.viewport = config_.viewport;
    auto result = engine_.navigate(request);
    std::unique_ptr<storage::StorageArea> area;
    if (!config_.storageDatabasePath.empty())
        area = openStorage(result.origin);
    return Page(std::move(result), std::move(area));
}

Page Browser::loadHtml(std::string source, std::string documentUrl) const {
    auto result = engine_.loadHtml(source, std::move(documentUrl), config_.viewport);
    std::unique_ptr<storage::StorageArea> area;
    if (!config_.storageDatabasePath.empty()) area = openStorage(result.origin);
    return Page(std::move(result), std::move(area));
}

Page Browser::navigate(std::string url) const {
    NavigationRequest request;
    request.url = std::move(url);
    return navigate(std::move(request));
}

std::unique_ptr<storage::StorageArea> Browser::openStorage(
    const security::Origin& origin) const {
    if (config_.storageDatabasePath.empty())
        throw BrowserError("Persistent storage is disabled: no profile database path configured");
    if (!origin.valid())
        throw BrowserError("Persistent storage requires a validated HTTP(S) origin");
    try {
        return std::make_unique<storage::StorageArea>(
            config_.storageDatabasePath, origin);
    } catch (const storage::StorageError& error) {
        throw BrowserError(std::string("Unable to open page storage: ") + error.what());
    }
}

} // namespace lapislazuli::browser
