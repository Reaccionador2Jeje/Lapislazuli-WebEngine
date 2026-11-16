#include "lapislazuli/storage/storage_area.hpp"

#include <sqlite3.h>

#include <limits>
#include <mutex>
#include <string>
#include <utility>

namespace lapislazuli::storage {
namespace {

void check(int result, sqlite3* db, const char* operation) {
    if (result != SQLITE_OK && result != SQLITE_DONE && result != SQLITE_ROW) {
        throw StorageError(std::string(operation) + ": " + (db ? sqlite3_errmsg(db) : "SQLite error"));
    }
}

class Statement {
public:
    Statement(sqlite3* db, const char* sql) : db_(db) {
        check(sqlite3_prepare_v2(db_, sql, -1, &statement_, nullptr), db_, "Prepare storage query");
    }
    ~Statement() { sqlite3_finalize(statement_); }
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    sqlite3_stmt* get() const noexcept { return statement_; }
    void bindText(int index, std::string_view value) {
        if (value.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw StorageError("Storage value is too large");
        }
        const char* data = value.empty() ? "" : value.data();
        check(sqlite3_bind_text(statement_, index, data, static_cast<int>(value.size()), SQLITE_TRANSIENT),
              db_, "Bind storage text");
    }
private:
    sqlite3* db_;
    sqlite3_stmt* statement_ = nullptr;
};

std::string columnText(sqlite3_stmt* statement, int column) {
    const auto* value = sqlite3_column_text(statement, column);
    const int bytes = sqlite3_column_bytes(statement, column);
    if (!value || bytes <= 0) return {};
    return {reinterpret_cast<const char*>(value), static_cast<std::size_t>(bytes)};
}

std::size_t scalarSize(sqlite3* db, const char* sql, std::string_view origin) {
    Statement statement(db, sql);
    statement.bindText(1, origin);
    const int result = sqlite3_step(statement.get());
    check(result, db, "Read storage accounting");
    const auto value = sqlite3_column_int64(statement.get(), 0);
    if (value < 0) throw StorageError("Invalid storage accounting value");
    return static_cast<std::size_t>(value);
}

} // namespace

struct StorageArea::Impl {
    sqlite3* db = nullptr;
    std::string origin;
    StorageConfig config;
    mutable std::mutex mutex;

    Impl(std::string path, std::string originValue, StorageConfig cfg)
        : origin(std::move(originValue)), config(cfg) {
        if (origin.empty() || origin.size() > 2048) {
            throw StorageError("Origin identifier must contain between 1 and 2048 bytes");
        }
        for (unsigned char c : origin) {
            if (c < 0x20 || c == 0x7f) throw StorageError("Origin identifier contains control characters");
        }
        if (config.quotaBytes == 0 || config.busyTimeoutMs < 0) {
            throw StorageError("Invalid storage configuration");
        }

        const int opened = sqlite3_open_v2(path.c_str(), &db,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
        if (opened != SQLITE_OK) {
            const std::string message = db ? sqlite3_errmsg(db) : "Unable to allocate SQLite database";
            if (db) sqlite3_close(db);
            db = nullptr;
            throw StorageError("Open storage database: " + message);
        }
        try {
            check(sqlite3_busy_timeout(db, config.busyTimeoutMs), db, "Configure storage busy timeout");
            char* error = nullptr;
            const int result = sqlite3_exec(db,
                "PRAGMA journal_mode=WAL;"
                "CREATE TABLE IF NOT EXISTS storage_items ("
                "origin TEXT NOT NULL, item_key TEXT NOT NULL, item_value TEXT NOT NULL,"
                "PRIMARY KEY(origin, item_key));",
                nullptr, nullptr, &error);
            if (result != SQLITE_OK) {
                const std::string message = error ? error : sqlite3_errmsg(db);
                sqlite3_free(error);
                throw StorageError("Initialize storage database: " + message);
            }
        } catch (...) {
            sqlite3_close(db);
            db = nullptr;
            throw;
        }
    }

    ~Impl() { if (db) sqlite3_close(db); }
};

StorageArea::StorageArea(std::string databasePath, std::string origin, StorageConfig config)
    : impl_(std::make_unique<Impl>(std::move(databasePath), std::move(origin), config)) {}

StorageArea::~StorageArea() = default;

std::optional<std::string> StorageArea::getItem(std::string_view keyValue) const {
    std::lock_guard lock(impl_->mutex);
    Statement statement(impl_->db,
        "SELECT item_value FROM storage_items WHERE origin=?1 AND item_key=?2");
    statement.bindText(1, impl_->origin);
    statement.bindText(2, keyValue);
    const int result = sqlite3_step(statement.get());
    if (result == SQLITE_DONE) return std::nullopt;
    check(result, impl_->db, "Read storage item");
    return columnText(statement.get(), 0);
}

void StorageArea::setItem(std::string_view keyValue, std::string_view value) {
    if (keyValue.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
        value.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw StorageError("Storage key or value is too large");
    }
    std::lock_guard lock(impl_->mutex);
    char* error = nullptr;
    if (sqlite3_exec(impl_->db, "BEGIN IMMEDIATE", nullptr, nullptr, &error) != SQLITE_OK) {
        const std::string message = error ? error : sqlite3_errmsg(impl_->db);
        sqlite3_free(error);
        throw StorageError("Begin storage transaction: " + message);
    }
    try {
        Statement previous(impl_->db,
            "SELECT length(CAST(item_value AS BLOB)) FROM storage_items WHERE origin=?1 AND item_key=?2");
        previous.bindText(1, impl_->origin);
        previous.bindText(2, keyValue);
        const int previousResult = sqlite3_step(previous.get());
        check(previousResult, impl_->db, "Read previous storage item");
        const bool exists = previousResult == SQLITE_ROW;
        const auto oldValueBytes = exists
            ? static_cast<std::size_t>(sqlite3_column_int64(previous.get(), 0)) : 0U;
        const auto current = scalarSize(impl_->db,
            "SELECT COALESCE(SUM(length(CAST(item_key AS BLOB)) + length(CAST(item_value AS BLOB))),0)"
            " FROM storage_items WHERE origin=?1", impl_->origin);
        const auto added = exists ? value.size() : keyValue.size() + value.size();
        if (current < oldValueBytes || added > impl_->config.quotaBytes ||
            current - oldValueBytes > impl_->config.quotaBytes - added) {
            throw StorageError("Origin storage quota exceeded");
        }

        Statement write(impl_->db,
            "INSERT INTO storage_items(origin,item_key,item_value) VALUES(?1,?2,?3)"
            " ON CONFLICT(origin,item_key) DO UPDATE SET item_value=excluded.item_value");
        write.bindText(1, impl_->origin);
        write.bindText(2, keyValue);
        write.bindText(3, value);
        check(sqlite3_step(write.get()), impl_->db, "Write storage item");
        if (sqlite3_exec(impl_->db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
            throw StorageError("Commit storage transaction: " + std::string(sqlite3_errmsg(impl_->db)));
        }
    } catch (...) {
        sqlite3_exec(impl_->db, "ROLLBACK", nullptr, nullptr, nullptr);
        throw;
    }
}

bool StorageArea::removeItem(std::string_view keyValue) {
    std::lock_guard lock(impl_->mutex);
    Statement statement(impl_->db, "DELETE FROM storage_items WHERE origin=?1 AND item_key=?2");
    statement.bindText(1, impl_->origin);
    statement.bindText(2, keyValue);
    check(sqlite3_step(statement.get()), impl_->db, "Remove storage item");
    return sqlite3_changes(impl_->db) > 0;
}

void StorageArea::clear() {
    std::lock_guard lock(impl_->mutex);
    Statement statement(impl_->db, "DELETE FROM storage_items WHERE origin=?1");
    statement.bindText(1, impl_->origin);
    check(sqlite3_step(statement.get()), impl_->db, "Clear origin storage");
}

std::optional<std::string> StorageArea::key(std::size_t index) const {
    std::lock_guard lock(impl_->mutex);
    Statement statement(impl_->db,
        "SELECT item_key FROM storage_items WHERE origin=?1 ORDER BY item_key COLLATE BINARY LIMIT 1 OFFSET ?2");
    statement.bindText(1, impl_->origin);
    if (index > static_cast<std::size_t>(std::numeric_limits<sqlite3_int64>::max())) {
        return std::nullopt;
    }
    check(sqlite3_bind_int64(statement.get(), 2, static_cast<sqlite3_int64>(index)),
          impl_->db, "Bind storage index");
    const int result = sqlite3_step(statement.get());
    if (result == SQLITE_DONE) return std::nullopt;
    check(result, impl_->db, "Read storage key");
    return columnText(statement.get(), 0);
}

std::size_t StorageArea::length() const {
    std::lock_guard lock(impl_->mutex);
    return scalarSize(impl_->db,
        "SELECT COUNT(*) FROM storage_items WHERE origin=?1", impl_->origin);
}

std::size_t StorageArea::usageBytes() const {
    std::lock_guard lock(impl_->mutex);
    return scalarSize(impl_->db,
        "SELECT COALESCE(SUM(length(CAST(item_key AS BLOB)) + length(CAST(item_value AS BLOB))),0)"
        " FROM storage_items WHERE origin=?1", impl_->origin);
}

const std::string& StorageArea::origin() const noexcept { return impl_->origin; }

} // namespace lapislazuli::storage
