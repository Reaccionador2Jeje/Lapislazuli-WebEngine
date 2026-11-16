#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace lapislazuli::security {

class SecurityError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Canonical tuple origin for HTTP(S) URLs. This intentionally supports a
// constrained ASCII host syntax; it is not a complete WHATWG URL/IDNA parser.
class Origin final {
public:
    Origin() = default;

    [[nodiscard]] static Origin fromUrl(std::string_view url);
    [[nodiscard]] bool valid() const noexcept { return !scheme_.empty(); }
    [[nodiscard]] const std::string& scheme() const noexcept { return scheme_; }
    [[nodiscard]] const std::string& host() const noexcept { return host_; }
    [[nodiscard]] std::uint16_t port() const noexcept { return port_; }
    [[nodiscard]] std::string serialize() const;
    [[nodiscard]] bool sameOrigin(const Origin& other) const noexcept;

private:
    Origin(std::string scheme, std::string host, std::uint16_t port);

    std::string scheme_;
    std::string host_;
    std::uint16_t port_ = 0;
};

} // namespace lapislazuli::security
