#pragma once

#include "lapislazuli/css/css_engine.hpp"
#include "lapislazuli/dom.hpp"
#include "lapislazuli/net/http_client.hpp"
#include "lapislazuli/layout/layout_engine.hpp"
#include "lapislazuli/render/display_list.hpp"
#include "lapislazuli/security/origin.hpp"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace lapislazuli::browser {

struct NavigationRequest {
    std::string url;
    std::map<std::string, std::string> headers;
    layout::Viewport viewport;
};

struct ScriptExecutionReport {
    std::size_t inlineScriptsFound = 0;
    std::size_t externalScriptsFound = 0;
    std::size_t scriptsExecuted = 0;
    std::vector<std::string> errors;
};

enum class ResourceKind { Stylesheet, Image, Script };

struct ResourceResult {
    ResourceKind kind = ResourceKind::Image;
    std::string url;
    long statusCode = 0;
    std::string contentType;
    bool loaded = false;
    std::string error;
    // Retained for images and fetched resources; stylesheet source is also parsed.
    std::string body;
};

struct NavigationResult {
    long statusCode = 0;
    std::string requestedUrl;
    std::string finalUrl;
    std::string contentType;
    security::Origin origin;
    bool redirected = false;
    std::unique_ptr<dom::Document> document;
    ScriptExecutionReport scripts;
    std::vector<ResourceResult> resources;
    std::vector<css::StyleSheet> styleSheets;
    std::unordered_map<const dom::Element*, css::ComputedStyle> computedStyles;
    layout::LayoutResult layoutTree;
    render::DisplayList displayList;
};

class NavigationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Engine {
public:
    explicit Engine(net::ClientConfig networkConfig = {});
    [[nodiscard]] NavigationResult navigate(const NavigationRequest& request) const;
    // Processes an already available HTML source through the same document pipeline.
    [[nodiscard]] NavigationResult loadHtml(std::string_view source, std::string documentUrl,
                                            layout::Viewport viewport = {}) const;
    // Executes supported classic scripts and loads linked stylesheets and images.
    [[nodiscard]] ScriptExecutionReport executeInlineScripts(dom::Document& document) const;
    // Resolves an HTTP(S) reference against an absolute HTTP(S) base URL.
    [[nodiscard]] static std::string resolveUrl(std::string_view base, std::string_view reference);

private:
    net::HttpClient http_;
};

} // namespace lapislazuli::browser
