#include "lapislazuli/js/runtime.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace lapislazuli::js {
namespace {

enum class Kind { End, Identifier, Number, String, Symbol };
struct Token { Kind kind; std::string text; };

class Lexer {
public:
    explicit Lexer(std::string_view source) : source_(source) {}

    std::vector<Token> scan() {
        std::vector<Token> tokens;
        while (position_ < source_.size()) {
            const char c = source_[position_];
            if (std::isspace(static_cast<unsigned char>(c))) { ++position_; continue; }
            if (c == '/' && position_ + 1 < source_.size() && source_[position_ + 1] == '/') {
                while (position_ < source_.size() && source_[position_] != '\n') ++position_;
                continue;
            }
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '$') {
                const auto start = position_++;
                while (position_ < source_.size()) {
                    const char n = source_[position_];
                    if (!std::isalnum(static_cast<unsigned char>(n)) && n != '_' && n != '$') break;
                    ++position_;
                }
                tokens.push_back({Kind::Identifier, std::string(source_.substr(start, position_ - start))});
                continue;
            }
            if (std::isdigit(static_cast<unsigned char>(c)) ||
                (c == '.' && position_ + 1 < source_.size() &&
                 std::isdigit(static_cast<unsigned char>(source_[position_ + 1])))) {
                const auto start = position_++;
                while (position_ < source_.size() &&
                       (std::isdigit(static_cast<unsigned char>(source_[position_])) ||
                        source_[position_] == '.')) ++position_;
                tokens.push_back({Kind::Number, std::string(source_.substr(start, position_ - start))});
                continue;
            }
            if (c == '"' || c == '\'') {
                const char quote = c;
                ++position_;
                std::string value;
                bool closed = false;
                while (position_ < source_.size()) {
                    char current = source_[position_++];
                    if (current == quote) { closed = true; break; }
                    if (current == '\\' && position_ < source_.size()) {
                        const char escaped = source_[position_++];
                        switch (escaped) {
                            case 'n': value.push_back('\n'); break;
                            case 'r': value.push_back('\r'); break;
                            case 't': value.push_back('\t'); break;
                            default: value.push_back(escaped); break;
                        }
                    } else value.push_back(current);
                }
                if (!closed) throw std::runtime_error("Unterminated JavaScript string");
                tokens.push_back({Kind::String, std::move(value)});
                continue;
            }
            static constexpr std::string_view doubleOps[] = {"==", "!=", "<=", ">=", "&&", "||"};
            bool matched = false;
            for (auto op : doubleOps) {
                if (source_.substr(position_, op.size()) == op) {
                    tokens.push_back({Kind::Symbol, std::string(op)});
                    position_ += op.size();
                    matched = true;
                    break;
                }
            }
            if (matched) continue;
            if (std::string_view("+-*/%()=,;!<>").find(c) != std::string_view::npos) {
                tokens.push_back({Kind::Symbol, std::string(1, c)});
                ++position_;
                continue;
            }
            throw std::runtime_error(std::string("Unsupported JavaScript character: ") + c);
        }
        tokens.push_back({Kind::End, {}});
        return tokens;
    }

private:
    std::string_view source_;
    std::size_t position_ = 0;
};

bool isNullish(const Value& value) {
    return std::holds_alternative<Null>(value.storage()) ||
           std::holds_alternative<Undefined>(value.storage());
}

bool isString(const Value& value) {
    return std::holds_alternative<std::string>(value.storage());
}

Value numericResult(double value) {
    if (!std::isfinite(value)) return Value(Null{});
    return Value(value);
}

class Evaluator {
public:
    Evaluator(std::vector<Token> tokens,
              std::unordered_map<std::string, Value>& globals,
              const std::unordered_map<std::string, Runtime::NativeFunction>& functions)
        : tokens_(std::move(tokens)), globals_(globals), functions_(functions) {}

    Value run() {
        Value last;
        while (!check(Kind::End)) {
            if (match(";")) continue;
            last = statement();
            match(";");
        }
        return last;
    }

private:
    const Token& current() const { return tokens_[position_]; }
    const Token& peek(std::size_t offset = 1) const {
        const auto index = position_ + offset;
        return tokens_[index < tokens_.size() ? index : tokens_.size() - 1];
    }
    bool check(Kind kind) const { return current().kind == kind; }
    bool check(std::string_view text) const { return current().text == text; }
    bool match(std::string_view text) {
        if (!check(text)) return false;
        ++position_;
        return true;
    }
    Token consume(Kind kind, std::string_view message) {
        if (!check(kind)) throw std::runtime_error(std::string(message));
        return tokens_[position_++];
    }
    void expect(std::string_view text) {
        if (!match(text)) throw std::runtime_error("Expected '" + std::string(text) + "'");
    }

    Value statement() {
        if (check(Kind::Identifier) &&
            (checkWord("let") || checkWord("const") || checkWord("var"))) {
            ++position_;
            const auto name = consume(Kind::Identifier, "Expected a variable name").text;
            Value value;
            if (match("=")) value = expression();
            globals_[name] = value;
            return value;
        }
        if (check(Kind::Identifier) && peek().text == "=") {
            const auto name = current().text;
            position_ += 2;
            Value value = expression();
            globals_[name] = value;
            return value;
        }
        return expression();
    }

    bool checkWord(std::string_view word) const {
        return check(Kind::Identifier) && current().text == word;
    }

    Value expression(int minimumPrecedence = 0) {
        Value left = unary();
        while (true) {
            const auto op = current().text;
            const int precedence = getPrecedence(op);
            if (precedence < minimumPrecedence) break;
            ++position_;
            Value right = expression(precedence + 1);
            left = binary(op, left, right);
        }
        return left;
    }

    static int getPrecedence(std::string_view op) {
        if (op == "||") return 1;
        if (op == "&&") return 2;
        if (op == "==" || op == "!=") return 3;
        if (op == "<" || op == ">" || op == "<=" || op == ">=") return 4;
        if (op == "+" || op == "-") return 5;
        if (op == "*" || op == "/" || op == "%") return 6;
        return -1;
    }

    Value unary() {
        if (match("!")) return Value(!unary().toBoolean());
        if (match("-")) return numericResult(-unary().toNumber());
        if (match("+")) return numericResult(unary().toNumber());
        return primary();
    }

    Value primary() {
        if (match("(")) {
            Value value = expression();
            expect(")");
            return value;
        }
        if (check(Kind::Number)) {
            const auto token = tokens_[position_++];
            char* end = nullptr;
            const double number = std::strtod(token.text.c_str(), &end);
            if (end == token.text.c_str() || *end != '\0' || !std::isfinite(number))
                throw std::runtime_error("Invalid numeric literal");
            return Value(number);
        }
        if (check(Kind::String)) return Value(tokens_[position_++].text);
        if (checkWord("true")) { ++position_; return Value(true); }
        if (checkWord("false")) { ++position_; return Value(false); }
        if (checkWord("null")) { ++position_; return Value(Null{}); }
        if (checkWord("undefined")) { ++position_; return Value(); }
        if (check(Kind::Identifier)) {
            const auto name = tokens_[position_++].text;
            if (match("(")) {
                std::vector<Value> arguments;
                if (!check(")")) {
                    do { arguments.push_back(expression()); } while (match(","));
                }
                expect(")");
                const auto function = functions_.find(name);
                if (function == functions_.end())
                    throw std::runtime_error("Unknown function: " + name);
                return function->second(arguments);
            }
            const auto value = globals_.find(name);
            if (value == globals_.end()) throw std::runtime_error("Undefined variable: " + name);
            return value->second;
        }
        throw std::runtime_error("Expected a JavaScript expression");
    }

    static Value binary(const std::string& op, const Value& left, const Value& right) {
        if (op == "+") {
            if (isString(left) || isString(right))
                return Value(left.toString() + right.toString());
            return numericResult(left.toNumber() + right.toNumber());
        }
        if (op == "-") return numericResult(left.toNumber() - right.toNumber());
        if (op == "*") return numericResult(left.toNumber() * right.toNumber());
        if (op == "/") {
            if (right.toNumber() == 0.0) throw std::runtime_error("Division by zero");
            return numericResult(left.toNumber() / right.toNumber());
        }
        if (op == "%") {
            if (right.toNumber() == 0.0) throw std::runtime_error("Division by zero");
            return numericResult(std::fmod(left.toNumber(), right.toNumber()));
        }
        if (op == "==") return Value(left.toString() == right.toString() || (isNullish(left) && isNullish(right)));
        if (op == "!=") return Value(!(left.toString() == right.toString() || (isNullish(left) && isNullish(right))));
        if (op == "<") return Value(left.toNumber() < right.toNumber());
        if (op == ">") return Value(left.toNumber() > right.toNumber());
        if (op == "<=") return Value(left.toNumber() <= right.toNumber());
        if (op == ">=") return Value(left.toNumber() >= right.toNumber());
        if (op == "&&") return Value(left.toBoolean() && right.toBoolean());
        if (op == "||") return Value(left.toBoolean() || right.toBoolean());
        throw std::runtime_error("Unsupported JavaScript operator: " + op);
    }

    std::vector<Token> tokens_;
    std::size_t position_ = 0;
    std::unordered_map<std::string, Value>& globals_;
    const std::unordered_map<std::string, Runtime::NativeFunction>& functions_;
};

} // namespace

std::string Value::toString() const {
    if (std::holds_alternative<Undefined>(value_)) return "undefined";
    if (std::holds_alternative<Null>(value_)) return "null";
    if (const auto* value = std::get_if<bool>(&value_)) return *value ? "true" : "false";
    if (const auto* value = std::get_if<double>(&value_)) {
        if (std::floor(*value) == *value) return std::to_string(static_cast<long long>(*value));
        std::ostringstream output;
        output << std::setprecision(15) << *value;
        return output.str();
    }
    return std::get<std::string>(value_);
}

double Value::toNumber() const noexcept {
    if (const auto* value = std::get_if<double>(&value_)) return *value;
    if (const auto* value = std::get_if<bool>(&value_)) return *value ? 1.0 : 0.0;
    if (std::holds_alternative<Null>(value_)) return 0.0;
    if (const auto* value = std::get_if<std::string>(&value_)) {
        if (value->empty()) return 0.0;
        char* end = nullptr;
        const double parsed = std::strtod(value->c_str(), &end);
        return end != value->c_str() && *end == '\0' ? parsed : 0.0;
    }
    return 0.0;
}

bool Value::toBoolean() const noexcept {
    if (std::holds_alternative<Undefined>(value_) || std::holds_alternative<Null>(value_)) return false;
    if (const auto* value = std::get_if<bool>(&value_)) return *value;
    if (const auto* value = std::get_if<double>(&value_)) return *value != 0.0 && !std::isnan(*value);
    if (const auto* value = std::get_if<std::string>(&value_)) return !value->empty();
    return false;
}

Runtime::Runtime() {
    registerFunction("String", [](const std::vector<Value>& args) {
        return args.empty() ? Value("undefined") : Value(args.front().toString());
    });
    registerFunction("Number", [](const std::vector<Value>& args) {
        return args.empty() ? Value(0) : numericResult(args.front().toNumber());
    });
    registerFunction("Boolean", [](const std::vector<Value>& args) {
        return Value(!args.empty() && args.front().toBoolean());
    });
    registerFunction("print", [this](const std::vector<Value>& args) {
        if (output_) {
            std::string line;
            for (std::size_t i = 0; i < args.size(); ++i) {
                if (i) line += " ";
                line += args[i].toString();
            }
            output_(line);
        }
        return Value();
    });
}

void Runtime::registerFunction(std::string name, NativeFunction function) {
    functions_[std::move(name)] = std::move(function);
}

void Runtime::setOutput(OutputFunction output) {
    output_ = std::move(output);
}

void Runtime::bindDocument(dom::Document& document) {
    registerFunction("getElementText", [&document](const std::vector<Value>& args) {
        if (args.empty()) return Value(Null{});
        const auto* element = document.getElementById(args.front().toString());
        return element ? Value(element->textContent()) : Value(Null{});
    });
    registerFunction("setElementText", [&document](const std::vector<Value>& args) {
        if (args.size() < 2) throw std::runtime_error("setElementText requires an id and text");
        auto* element = document.getElementById(args[0].toString());
        if (!element) return Value(false);
        element->setTextContent(args[1].toString());
        return Value(true);
    });
    registerFunction("setElementAttribute", [&document](const std::vector<Value>& args) {
        if (args.size() < 3)
            throw std::runtime_error("setElementAttribute requires an id, name, and value");
        auto* element = document.getElementById(args[0].toString());
        if (!element) return Value(false);
        element->setAttribute(args[1].toString(), args[2].toString());
        return Value(true);
    });
    registerFunction("getElementAttribute", [&document](const std::vector<Value>& args) {
        if (args.size() < 2) return Value(Null{});
        const auto* element = document.getElementById(args[0].toString());
        if (!element) return Value(Null{});
        const auto* value = element->getAttribute(args[1].toString());
        return value ? Value(*value) : Value(Null{});
    });
    registerFunction("removeElementAttribute", [&document](const std::vector<Value>& args) {
        if (args.size() < 2) throw std::runtime_error("removeElementAttribute requires an id and name");
        auto* element = document.getElementById(args[0].toString());
        return Value(element && element->removeAttribute(args[1].toString()));
    });
    registerFunction("clickElement", [this, &document](const std::vector<Value>& args) {
        if (args.empty()) throw std::runtime_error("clickElement requires an element id");
        const auto* element = document.getElementById(args.front().toString());
        if (!element) return Value(false);
        const auto* handler = element->getAttribute("onclick");
        if (!handler || handler->empty()) return Value(false);
        (void)execute(*handler);
        return Value(true);
    });
}

Value Runtime::execute(std::string_view source) {
    Evaluator evaluator(Lexer(source).scan(), globals_, functions_);
    return evaluator.run();
}

} // namespace lapislazuli::js
