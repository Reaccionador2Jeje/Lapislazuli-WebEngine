#pragma once

#include "lapislazuli/dom.hpp"
#include "lapislazuli/net/http_client.hpp"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace lapislazuli::browser {

struct NavigationRequest {
    std::string url;
    std::map<std::string, std::string> headers;
};

struct ScriptExecutionReport {
    std::size_t inlineScriptsFound = 0;
    std::size_t scriptsExecuted = 0;
    std::vector<std::string> errors;
};

struct NavigationResult {
    long statusCode = 0;
    std::string requestedUrl;
    std::string finalUrl;
    std::string contentType;
    bool redirected = false;
    std::unique_ptr<dom::Document> document;
    ScriptExecutionReport scripts;
};

class NavigationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Small browser-facing facade. It fetches an HTML response and builds the
// current experimental DOM; it is not yet a complete browser navigation stack.
class Engine {
public:
    explicit Engine(net::ClientConfig networkConfig = {});
    [[nodiscard]] NavigationResult navigate(const NavigationRequest& request) const;
    // Executes supported inline classic scripts in document order. External and module scripts are skipped.
    [[nodiscard]] ScriptExecutionReport executeInlineScripts(dom::Document& document) const;

private:
    net::HttpClient http_;
};

} // namespace lapislazuli::browser
