#pragma once

#include "lapislazuli/browser/engine.hpp"
#include "lapislazuli/storage/storage_area.hpp"

#include <memory>
#include <stdexcept>
#include <string>

namespace lapislazuli::browser {

struct BrowserConfig {
    net::ClientConfig network;
    std::string storageDatabasePath;
    layout::Viewport viewport;
};

class BrowserError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// A navigated page and its origin-partitioned storage area.
class Page final {
public:
    Page(NavigationResult navigation, std::unique_ptr<storage::StorageArea> storage);

    Page(const Page&) = delete;
    Page& operator=(const Page&) = delete;
    Page(Page&&) noexcept = default;
    Page& operator=(Page&&) noexcept = default;

    [[nodiscard]] const NavigationResult& navigation() const noexcept { return navigation_; }
    [[nodiscard]] const security::Origin& origin() const noexcept { return navigation_.origin; }
    [[nodiscard]] const dom::Document& document() const;
    [[nodiscard]] dom::Document& document();
    [[nodiscard]] const layout::LayoutResult& layoutTree() const noexcept {
        return navigation_.layoutTree;
    }
    [[nodiscard]] const render::DisplayList& displayList() const noexcept {
        return navigation_.displayList;
    }
    [[nodiscard]] const std::vector<ResourceResult>& resources() const noexcept {
        return navigation_.resources;
    }
    // Returns nullptr when persistent storage is disabled in BrowserConfig.
    [[nodiscard]] storage::StorageArea* storage() noexcept { return storage_.get(); }
    [[nodiscard]] const storage::StorageArea* storage() const noexcept { return storage_.get(); }

private:
    NavigationResult navigation_;
    std::unique_ptr<storage::StorageArea> storage_;
};

// Reusable embedder-facing browser context. Storage is shared across pages
// through one profile database and partitioned by validated tuple origin.
class Browser final {
public:
    explicit Browser(BrowserConfig config = {});

    [[nodiscard]] Page navigate(NavigationRequest request) const;
    [[nodiscard]] Page navigate(std::string url) const;
    [[nodiscard]] Page loadHtml(std::string source, std::string documentUrl) const;
    [[nodiscard]] std::unique_ptr<storage::StorageArea> openStorage(
        const security::Origin& origin) const;

private:
    BrowserConfig config_;
    Engine engine_;
};

} // namespace lapislazuli::browser
