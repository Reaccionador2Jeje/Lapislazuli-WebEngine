#include "lapislazuli/browser/engine.hpp"
#include "lapislazuli/dom_parser.hpp"
#include "lapislazuli/js/runtime.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <string_view>
#include <utility>

namespace lapislazuli::browser {
namespace {

struct ParsedUrl {
    std::string scheme;
    std::string authority;
    std::string path;
    std::string query;
};

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string normalizedMimeType(std::string value) {
    const auto separator = value.find(';');
    if (separator != std::string::npos) value.resize(separator);
    const auto first = value.find_first_not_of(" \t");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t");
    return lower(value.substr(first, last - first + 1));
}

ParsedUrl parseAbsoluteUrl(std::string_view value) {
    const auto schemeEnd = value.find("://");
    if (schemeEnd == std::string_view::npos)
        throw NavigationError("Base URL must be absolute and include a scheme");

    ParsedUrl result;
    result.scheme = lower(std::string(value.substr(0, schemeEnd)));
    const auto authorityStart = schemeEnd + 3;
    const auto authorityEnd = value.find_first_of("/?#", authorityStart);
    result.authority = std::string(value.substr(authorityStart,
        (authorityEnd == std::string_view::npos ? value.size() : authorityEnd) - authorityStart));

    std::size_t pathStart = authorityEnd == std::string_view::npos ? value.size() : authorityEnd;
    if (pathStart < value.size() && value[pathStart] == '/') {
        const auto pathEnd = value.find_first_of("?#", pathStart);
        result.path = std::string(value.substr(pathStart,
            (pathEnd == std::string_view::npos ? value.size() : pathEnd) - pathStart));
        pathStart = pathEnd == std::string_view::npos ? value.size() : pathEnd;
    } else {
        result.path = "/";
    }
    if (pathStart < value.size() && value[pathStart] == '?') {
        const auto queryEnd = value.find('#', pathStart);
        result.query = std::string(value.substr(pathStart,
            (queryEnd == std::string_view::npos ? value.size() : queryEnd) - pathStart));
    }
    if (result.authority.empty()) throw NavigationError("URL authority is missing");
    return result;
}

std::string normalizePath(std::string path) {
    const bool trailingSlash = !path.empty() && path.back() == '/';
    std::vector<std::string> segments;
    std::size_t cursor = 0;
    while (cursor <= path.size()) {
        const auto end = path.find('/', cursor);
        const auto part = path.substr(cursor,
            (end == std::string::npos ? path.size() : end) - cursor);
        if (part == "..") {
            if (!segments.empty()) segments.pop_back();
        } else if (!part.empty() && part != ".") {
            segments.push_back(part);
        }
        if (end == std::string::npos) break;
        cursor = end + 1;
    }
    std::string result = "/";
    for (std::size_t i = 0; i < segments.size(); ++i) {
        if (i) result += '/';
        result += segments[i];
    }
    if (trailingSlash && result.back() != '/') result += '/';
    return result;
}

bool containsToken(std::string_view value, std::string_view wanted) {
    std::size_t cursor = 0;
    while (cursor < value.size()) {
        while (cursor < value.size() &&
               std::isspace(static_cast<unsigned char>(value[cursor]))) ++cursor;
        const auto start = cursor;
        while (cursor < value.size() &&
               !std::isspace(static_cast<unsigned char>(value[cursor]))) ++cursor;
        if (lower(std::string(value.substr(start, cursor - start))) == wanted) return true;
    }
    return false;
}

bool supportedScriptType(const dom::Element& element) {
    const auto* type = element.getAttribute("type");
    if (!type || type->empty()) return true;
    const auto normalized = normalizedMimeType(*type);
    return normalized == "text/javascript" || normalized == "application/javascript" ||
           normalized == "text/ecmascript" || normalized == "application/ecmascript";
}

struct ResourceBudget {
    std::size_t count = 0;
    std::size_t bytes = 0;
    static constexpr std::size_t maxCount = 64;
    static constexpr std::size_t maxBytes = 32 * 1024 * 1024;
};

void processDocumentNode(dom::Node& node, const std::string& baseUrl,
                         const net::HttpClient& http, js::Runtime& runtime,
                         NavigationResult& result, ResourceBudget& budget) {
    for (const auto& childOwner : node.children()) {
        auto& child = *childOwner;
        if (child.type() != dom::NodeType::Element) {
            processDocumentNode(child, baseUrl, http, runtime, result, budget);
            continue;
        }

        auto& element = static_cast<dom::Element&>(child);
        const auto& tag = element.name();

        if (tag == "style") {
            result.styleSheets.push_back(css::Parser{}.parse(element.textContent()));
        } else if (tag == "link") {
            const auto* rel = element.getAttribute("rel");
            const auto* href = element.getAttribute("href");
            if (rel && href && containsToken(*rel, "stylesheet")) {
                ResourceResult item;
                item.kind = ResourceKind::Stylesheet;
                try {
                    if (budget.count >= ResourceBudget::maxCount)
                        throw NavigationError("Document resource-count limit exceeded");
                    ++budget.count;
                    item.url = Engine::resolveUrl(baseUrl, *href);
                    net::Request request;
                    request.url = item.url;
                    const auto response = http.perform(request);
                    item.statusCode = response.statusCode;
                    item.contentType = response.contentType;
                    if (response.statusCode < 200 || response.statusCode >= 300)
                        throw NavigationError("Stylesheet request returned HTTP " +
                                              std::to_string(response.statusCode));
                    if (normalizedMimeType(response.contentType) != "text/css")
                        throw NavigationError("Stylesheet response did not use text/css");
                    if (response.body.size() > ResourceBudget::maxBytes -
                        std::min(budget.bytes, ResourceBudget::maxBytes))
                        throw NavigationError("Document resource-byte budget exceeded");
                    budget.bytes += response.body.size();
                    item.body = response.body;
                    item.loaded = true;
                    result.styleSheets.push_back(css::Parser{}.parse(response.body));
                } catch (const std::exception& error) {
                    item.error = error.what();
                }
                result.resources.push_back(std::move(item));
            }
        } else if (tag == "img") {
            const auto* src = element.getAttribute("src");
            if (src && !src->empty()) {
                ResourceResult item;
                item.kind = ResourceKind::Image;
                try {
                    if (budget.count >= ResourceBudget::maxCount)
                        throw NavigationError("Document resource-count limit exceeded");
                    ++budget.count;
                    item.url = Engine::resolveUrl(baseUrl, *src);
                    net::Request request;
                    request.url = item.url;
                    const auto response = http.perform(request);
                    item.statusCode = response.statusCode;
                    item.contentType = response.contentType;
                    if (response.statusCode < 200 || response.statusCode >= 300)
                        throw NavigationError("Image request returned HTTP " +
                                              std::to_string(response.statusCode));
                    if (normalizedMimeType(response.contentType).rfind("image/", 0) != 0)
                        throw NavigationError("Image response did not use an image MIME type");
                    if (response.body.size() > ResourceBudget::maxBytes -
                        std::min(budget.bytes, ResourceBudget::maxBytes))
                        throw NavigationError("Document resource-byte budget exceeded");
                    budget.bytes += response.body.size();
                    item.body = response.body;
                    item.loaded = true;
                } catch (const std::exception& error) {
                    item.error = error.what();
                }
                result.resources.push_back(std::move(item));
            }
        } else if (tag == "script" && supportedScriptType(element)) {
            const auto* src = element.getAttribute("src");
            if (!src) {
                ++result.scripts.inlineScriptsFound;
                try {
                    (void)runtime.execute(element.textContent());
                    ++result.scripts.scriptsExecuted;
                } catch (const std::exception& error) {
                    result.scripts.errors.push_back(error.what());
                }
            } else {
                ++result.scripts.externalScriptsFound;
                if (element.hasAttribute("async") || element.hasAttribute("defer")) {
                    result.scripts.errors.push_back(
                        "External async/defer scripts are not supported yet: " + *src);
                } else {
                    ResourceResult item;
                    item.kind = ResourceKind::Script;
                    try {
                        if (budget.count >= ResourceBudget::maxCount)
                            throw NavigationError("Document resource-count limit exceeded");
                        ++budget.count;
                        item.url = Engine::resolveUrl(baseUrl, *src);
                        net::Request request;
                        request.url = item.url;
                        const auto response = http.perform(request);
                        item.statusCode = response.statusCode;
                        item.contentType = response.contentType;
                        if (response.statusCode < 200 || response.statusCode >= 300)
                            throw NavigationError("Script request returned HTTP " +
                                                  std::to_string(response.statusCode));
                        const auto mime = normalizedMimeType(response.contentType);
                        if (mime != "text/javascript" && mime != "application/javascript" &&
                            mime != "text/ecmascript" && mime != "application/ecmascript")
                            throw NavigationError("Script response has an unsupported JavaScript MIME type");
                        if (response.body.size() > ResourceBudget::maxBytes -
                            std::min(budget.bytes, ResourceBudget::maxBytes))
                            throw NavigationError("Document resource-byte budget exceeded");
                        budget.bytes += response.body.size();
                        item.body = response.body;
                        item.loaded = true;
                        try {
                            (void)runtime.execute(response.body);
                            ++result.scripts.scriptsExecuted;
                        } catch (const std::exception& error) {
                            result.scripts.errors.push_back(error.what());
                        }
                    } catch (const std::exception& error) {
                        item.error = error.what();
                        result.scripts.errors.push_back(item.error);
                    }
                    result.resources.push_back(std::move(item));
                }
            }
        }

        processDocumentNode(child, baseUrl, http, runtime, result, budget);
    }
}

} // namespace

Engine::Engine(net::ClientConfig networkConfig) : http_(networkConfig) {}

std::string Engine::resolveUrl(std::string_view base, std::string_view reference) {
    if (base.empty())
        throw NavigationError("Base URL must be non-empty");
    if (reference.find_first_of("\r\n\t ") != std::string_view::npos)
        throw NavigationError("Resource URL contains forbidden whitespace");

    net::HttpClient::validateUrl(base);
    const auto colon = reference.find(':');
    const auto firstDelimiter = reference.find_first_of("/?#");
    if (colon != std::string_view::npos &&
        (firstDelimiter == std::string_view::npos || colon < firstDelimiter) &&
        reference.find("://") == std::string_view::npos) {
        throw NavigationError("Resource URL uses an unsupported or malformed scheme");
    }
    if (reference.find("://") != std::string_view::npos) {
        net::HttpClient::validateUrl(reference);
        return std::string(reference);
    }

    const auto baseParts = parseAbsoluteUrl(base);
    std::string ref(reference);
    const auto fragmentAt = ref.find('#');
    const std::string fragment = fragmentAt == std::string::npos ? "" : ref.substr(fragmentAt);
    if (fragmentAt != std::string::npos) ref.resize(fragmentAt);

    std::string path;
    std::string query;
    if (ref.rfind("//", 0) == 0) {
        const auto absolute = baseParts.scheme + ":" + ref + fragment;
        net::HttpClient::validateUrl(absolute);
        return absolute;
    }

    if (ref.empty()) {
        path = baseParts.path;
        query = baseParts.query;
    } else if (ref.front() == '?') {
        path = baseParts.path;
        query = ref;
    } else {
        const auto queryAt = ref.find('?');
        const std::string refPath = ref.substr(0, queryAt);
        if (queryAt != std::string::npos) query = ref.substr(queryAt);
        if (!refPath.empty() && refPath.front() == '/') {
            path = normalizePath(refPath);
        } else {
            const auto slash = baseParts.path.rfind('/');
            const auto directory = slash == std::string::npos ? "/" :
                baseParts.path.substr(0, slash + 1);
            path = normalizePath(directory + refPath);
        }
    }

    const auto resolved = baseParts.scheme + "://" + baseParts.authority +
        (path.empty() ? "/" : path) + query + fragment;
    net::HttpClient::validateUrl(resolved);
    return resolved;
}

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
    result.finalUrl = response.effectiveUrl.empty() ? request.url : response.effectiveUrl;
    result.origin = security::Origin::fromUrl(result.finalUrl);
    result.contentType = response.contentType;
    result.redirected = response.redirected;
    result.document = parser.parse(response.body);

    std::string resourceBase = result.finalUrl;
    const auto bases = result.document->getElementsByTagName("base");
    if (!bases.empty()) {
        if (const auto* href = bases.front()->getAttribute("href")) {
            try {
                resourceBase = resolveUrl(resourceBase, *href);
            } catch (const std::exception&) {
                // An invalid base element is ignored; the final response URL remains authoritative.
            }
        }
    }

    js::Runtime runtime;
    runtime.bindDocument(*result.document);
    ResourceBudget budget;
    processDocumentNode(*result.document, resourceBase, http_, runtime, result, budget);

    std::vector<const css::StyleSheet*> styleSheets;
    styleSheets.reserve(result.styleSheets.size());
    for (const auto& sheet : result.styleSheets) styleSheets.push_back(&sheet);
    result.computedStyles = css::StyleResolver{}.computeTree(*result.document, styleSheets);
    result.layoutTree = layout::LayoutEngine{}.layout(
        *result.document, result.computedStyles, request.viewport);
    result.displayList = render::DisplayListBuilder{}.build(
        result.layoutTree, result.computedStyles);
    return result;
}

ScriptExecutionReport Engine::executeInlineScripts(dom::Document& document) const {
    ScriptExecutionReport report;
    js::Runtime runtime;
    runtime.bindDocument(document);

    for (auto* script : document.getElementsByTagName("script")) {
        if (script->hasAttribute("src") || !supportedScriptType(*script)) continue;

        ++report.inlineScriptsFound;
        try {
            (void)runtime.execute(script->textContent());
            ++report.scriptsExecuted;
        } catch (const std::exception& error) {
            report.errors.push_back(error.what());
        }
    }
    return report;
}

} // namespace lapislazuli::browser
