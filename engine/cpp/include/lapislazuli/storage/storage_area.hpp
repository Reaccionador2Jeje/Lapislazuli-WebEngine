#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace lapislazuli::storage {

class StorageError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct StorageConfig {
    std::size_t quotaBytes = 5U * 1024U * 1024U;
    int busyTimeoutMs = 5000;
};

// Persistent, origin-partitioned key/value storage.
// The embedder must supply a canonical, trusted origin identifier.
class StorageArea {
public:
    StorageArea(std::string databasePath, std::string origin, StorageConfig config = {});
    ~StorageArea();

    StorageArea(const StorageArea&) = delete;
    StorageArea& operator=(const StorageArea&) = delete;
    StorageArea(StorageArea&&) = delete;
    StorageArea& operator=(StorageArea&&) = delete;

    [[nodiscard]] std::optional<std::string> getItem(std::string_view key) const;
    void setItem(std::string_view key, std::string_view value);
    [[nodiscard]] bool removeItem(std::string_view key);
    void clear();
    [[nodiscard]] std::optional<std::string> key(std::size_t index) const;
    [[nodiscard]] std::size_t length() const;
    [[nodiscard]] std::size_t usageBytes() const;
    [[nodiscard]] const std::string& origin() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace lapislazuli::storage
