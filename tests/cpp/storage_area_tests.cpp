#include "lapislazuli/storage/storage_area.hpp"
#include "lapislazuli/security/origin.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

using lapislazuli::storage::StorageArea;
using lapislazuli::storage::StorageConfig;
using lapislazuli::storage::StorageError;

template <typename Function>
void rejects(Function&& function) {
    bool caught = false;
    try { function(); } catch (const StorageError&) { caught = true; }
    assert(caught);
}

int main() {
    const auto path = std::filesystem::temp_directory_path() / "lapislazuli-storage-tests.sqlite";
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    std::filesystem::remove(path.string() + "-wal", ignored);
    std::filesystem::remove(path.string() + "-shm", ignored);

    {
        StorageArea first(path.string(), lapislazuli::security::Origin::fromUrl("HTTPS://Alpha.Example:443/path"));
        first.setItem("theme", "dark");
        first.setItem("language", "es");
        first.setItem("empty", "");
        assert(first.getItem("theme") == std::optional<std::string>("dark"));
        assert(first.getItem("empty") == std::optional<std::string>(""));
        assert(first.length() == 3);
        assert(first.key(0) == std::optional<std::string>("empty"));
        assert(first.key(1) == std::optional<std::string>("language"));
        assert(first.key(2) == std::optional<std::string>("theme"));
        assert(!first.key(3).has_value());
        assert(first.usageBytes() == 24);

        StorageArea isolated(path.string(), "https://beta.example");
        assert(isolated.length() == 0);
        isolated.setItem("theme", "light");
        assert(isolated.getItem("theme") == std::optional<std::string>("light"));
        assert(first.getItem("theme") == std::optional<std::string>("dark"));
        assert(first.removeItem("language"));
        assert(!first.removeItem("missing"));
    }

    {
        StorageArea reopened(path.string(), lapislazuli::security::Origin::fromUrl("https://alpha.example/another-page"));
        assert(reopened.getItem("theme") == std::optional<std::string>("dark"));
        StorageConfig smallQuota;
        smallQuota.quotaBytes = 8;
        StorageArea limited(path.string(), "https://quota.example", smallQuota);
        limited.setItem("a", "123");
        rejects([&] { limited.setItem("a", "12345678"); });
        assert(limited.getItem("a") == std::optional<std::string>("123"));
        limited.clear();
        assert(limited.length() == 0);
    }

    rejects([&] { StorageArea invalid(path.string(), ""); });
    std::filesystem::remove(path, ignored);
    std::filesystem::remove(path.string() + "-wal", ignored);
    std::filesystem::remove(path.string() + "-shm", ignored);
    std::cout << "Persistent origin storage tests passed.\n";
}
