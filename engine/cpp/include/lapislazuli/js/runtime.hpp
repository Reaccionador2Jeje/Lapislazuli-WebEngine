#pragma once

#include "lapislazuli/dom.hpp"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace lapislazuli::js {

struct Undefined {};
struct Null {};

class Value {
public:
    using Storage = std::variant<Undefined, Null, bool, double, std::string>;

    Value() : value_(Undefined{}) {}
    Value(Null value) : value_(value) {}
    Value(bool value) : value_(value) {}
    Value(double value) : value_(value) {}
    Value(int value) : value_(static_cast<double>(value)) {}
    Value(std::string value) : value_(std::move(value)) {}
    Value(const char* value) : value_(std::string(value)) {}

    [[nodiscard]] const Storage& storage() const noexcept { return value_; }
    [[nodiscard]] std::string toString() const;
    [[nodiscard]] double toNumber() const noexcept;
    [[nodiscard]] bool toBoolean() const noexcept;

private:
    Storage value_;
};

class Runtime {
public:
    using NativeFunction = std::function<Value(const std::vector<Value>&)>;
    using OutputFunction = std::function<void(std::string_view)>;

    Runtime();

    // Registers a host-provided function callable from scripts.
    void registerFunction(std::string name, NativeFunction function);
    void setOutput(OutputFunction output);

    // Exposes a small, read-only DOM bridge: getElementText(id).
    void bindDocument(const dom::Document& document);

    // Executes the supported JavaScript subset. Throws std::runtime_error on errors.
    [[nodiscard]] Value execute(std::string_view source);
    [[nodiscard]] const std::unordered_map<std::string, Value>& globals() const noexcept {
        return globals_;
    }

private:
    std::unordered_map<std::string, Value> globals_;
    std::unordered_map<std::string, NativeFunction> functions_;
    OutputFunction output_;
};

} // namespace lapislazuli::js
