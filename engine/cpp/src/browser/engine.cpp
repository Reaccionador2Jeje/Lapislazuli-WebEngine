#include "lapislazuli/browser/engine.hpp"
#include "lapislazuli/dom_parser.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>

namespace lapislazuli::browser {
namespace {

std::string normalizedMimeType(std::string value) {
    const auto separator = value.find(';');
    if (separator != std::string::npos) value.resize(separator);
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const auto first = value.find_first_not_of(" \t");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t");
    return value.substr(first, last - first + 1);
}

} // namespace

Engine::Engine(net::ClientConfig networkConfig) : http_(networkConfig) {}

NavigationResult Engine::navigate(const NavigationRequest& request) const {
    if (request.url.empty()) throw NavigationError("Navigation URL cannot be empty");

    net::Request networkRequest;
    networkRequest.method = net::Method::Get;
    networkRequest.url = request.url;
    networkRequest.headers = request.headers;
    const auto response = http_.perform(networkRequest);

    const auto mime = normalizedMimeType(response.contentType);
    if (mime != "text/html" && mime != "application/xhtml+xml") {
        throw NavigationError("The response is not an HTML document (Content-Type: " +
                              (mime.empty() ? std::string("missing") : mime) + ")");
    }

    html::DocumentParser parser;
    NavigationResult result;
    result.statusCode = response.statusCode;
    result.requestedUrl = request.url;
    result.finalUrl = response.effectiveUrl;
    result.contentType = response.contentType;
    result.redirected = response.redirected;
    result.document = parser.parse(response.body);
    return result;
}

} // namespace lapislazuli::browser
