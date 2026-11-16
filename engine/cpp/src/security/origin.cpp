#include "lapislazuli/security/origin.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <string>

namespace lapislazuli::security {
namespace {

std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool validDnsHost(std::string_view host) {
    if (host.empty() || host.size() > 253) return false;

    const bool numericAddress = std::all_of(host.begin(), host.end(),
        [](unsigned char c) { return std::isdigit(c) || c == '.'; });
    if (numericAddress) {
        unsigned parts = 0;
        std::size_t cursor = 0;
        while (cursor <= host.size()) {
            const auto end = host.find('.', cursor);
            const auto part = host.substr(cursor,
                (end == std::string_view::npos ? host.size() : end) - cursor);
            if (part.empty() || part.size() > 3 ||
                (part.size() > 1 && part.front() == '0')) return false;
            unsigned value = 0;
            const auto parsed = std::from_chars(part.data(), part.data() + part.size(), value);
            if (parsed.ec != std::errc{} || parsed.ptr != part.data() + part.size() ||
                value > 255) return false;
            ++parts;
            if (end == std::string_view::npos) break;
            cursor = end + 1;
        }
        return parts == 4;
    }

    std::size_t cursor = 0;
    while (cursor < host.size()) {
        const auto end = host.find('.', cursor);
        const auto label = host.substr(cursor,
            (end == std::string_view::npos ? host.size() : end) - cursor);
        if (label.empty() || label.size() > 63 || label.front() == '-' ||
            label.back() == '-') return false;
        for (unsigned char c : label) {
            if (!(std::isalnum(c) || c == '-')) return false;
        }
        if (end == std::string_view::npos) break;
        cursor = end + 1;
    }
    return true;
}

std::uint16_t parsePort(std::string_view text) {
    if (text.empty() || text.size() > 5)
        throw SecurityError("URL port must be between 1 and 65535");
    unsigned value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
        value == 0 || value > 65535)
        throw SecurityError("URL port must be between 1 and 65535");
    return static_cast<std::uint16_t>(value);
}

} // namespace

Origin::Origin(std::string scheme, std::string host, std::uint16_t port)
    : scheme_(std::move(scheme)), host_(std::move(host)), port_(port) {}

Origin Origin::fromUrl(std::string_view url) {
    if (url.empty() || url.size() > 8192 ||
        url.find_first_of("\r\n\t ") != std::string_view::npos)
        throw SecurityError("Invalid URL length or whitespace");

    const auto schemeEnd = url.find("://");
    if (schemeEnd == std::string_view::npos)
        throw SecurityError("Origin requires an absolute HTTP(S) URL");

    const auto scheme = lowerAscii(std::string(url.substr(0, schemeEnd)));
    if (scheme != "http" && scheme != "https")
        throw SecurityError("Only HTTP and HTTPS URLs have tuple origins");

    const auto authorityStart = schemeEnd + 3;
    const auto authorityEnd = url.find_first_of("/?#", authorityStart);
    auto authority = url.substr(authorityStart,
        (authorityEnd == std::string_view::npos ? url.size() : authorityEnd) - authorityStart);
    if (authority.empty() || authority.find('@') != std::string_view::npos)
        throw SecurityError("URL authority is missing or contains forbidden user-info");

    std::string host;
    std::string_view portText;
    if (authority.front() == '[') {
        const auto close = authority.find(']');
        if (close == std::string_view::npos)
            throw SecurityError("Malformed bracketed IPv6 host");
        const auto address = authority.substr(1, close - 1);
        if (address.empty() || address.find(':') == std::string_view::npos)
            throw SecurityError("Malformed bracketed IPv6 host");
        for (unsigned char c : address) {
            if (!(std::isxdigit(c) || c == ':' || c == '.'))
                throw SecurityError("Unsupported character in IPv6 host");
        }
        host = "[" + lowerAscii(std::string(address)) + "]";
        const auto suffix = authority.substr(close + 1);
        if (!suffix.empty()) {
            if (suffix.front() != ':') throw SecurityError("Malformed URL authority");
            portText = suffix.substr(1);
        }
    } else {
        const auto colon = authority.rfind(':');
        if (colon != std::string_view::npos) {
            if (authority.find(':') != colon)
                throw SecurityError("IPv6 URL hosts must use brackets");
            portText = authority.substr(colon + 1);
            authority = authority.substr(0, colon);
        }
        std::string normalized = lowerAscii(std::string(authority));
        if (!normalized.empty() && normalized.back() == '.') normalized.pop_back();
        if (!validDnsHost(normalized))
            throw SecurityError("URL host is not a supported ASCII DNS or IPv4 host");
        host = std::move(normalized);
    }

    const std::uint16_t defaultPort = scheme == "https" ? 443 : 80;
    const auto port = portText.empty() ? defaultPort : parsePort(portText);
    return Origin(scheme, std::move(host), port);
}

std::string Origin::serialize() const {
    if (!valid()) return {};
    std::string result = scheme_ + "://" + host_;
    const auto defaultPort = scheme_ == "https" ? 443 : 80;
    if (port_ != defaultPort) result += ":" + std::to_string(port_);
    return result;
}

bool Origin::sameOrigin(const Origin& other) const noexcept {
    return valid() && other.valid() && scheme_ == other.scheme_ &&
           host_ == other.host_ && port_ == other.port_;
}

} // namespace lapislazuli::security
