#pragma once

#include "lapislazuli/dom.hpp"
#include "lapislazuli/net/http_client.hpp"

#include <map>
#include <memory>
#include <string>

namespace lapislazuli::browser {

struct NavigationRequest {
    std::string url;
    std::map<std::string, std::string> headers;
};

struct NavigationResult {
    long statusCode = 0;
    std::string requestedUrl;
    std::string finalUrl;
    std::string contentType;
    bool redirected = false;
    std::unique_ptr<dom::Document> document;
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

private:
    net::HttpClient http_;
};

} // namespace lapislazuli::browser
