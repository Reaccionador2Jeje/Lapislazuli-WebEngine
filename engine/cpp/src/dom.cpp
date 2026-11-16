#include "lapislazuli/dom.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace lapislazuli::dom {
namespace {

std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string escapeText(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            default: result += ch; break;
        }
    }
    return result;
}

std::string escapeAttribute(std::string_view value) {
    std::string result = escapeText(value);
    std::string escaped;
    escaped.reserve(result.size());
    for (const char ch : result) {
        if (ch == '"') escaped += "&quot;";
        else escaped += ch;
    }
    return escaped;
}

bool isVoidElement(std::string_view tag) {
    static constexpr std::string_view voidTags[] = {
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr"
    };
    return std::find(std::begin(voidTags), std::end(voidTags), tag) != std::end(voidTags);
}

void collectByTag(Node& node, std::string_view wanted, std::vector<Element*>& result) {
    for (const auto& child : node.children()) {
        if (child->type() == NodeType::Element) {
            auto* element = static_cast<Element*>(child.get());
            if (wanted == "*" || element->name() == wanted) result.push_back(element);
        }
        collectByTag(*child, wanted, result);
    }
}

void collectByTag(const Node& node, std::string_view wanted,
                  std::vector<const Element*>& result) {
    for (const auto& child : node.children()) {
        if (child->type() == NodeType::Element) {
            const auto* element = static_cast<const Element*>(child.get());
            if (wanted == "*" || element->name() == wanted) result.push_back(element);
        }
        collectByTag(*child, wanted, result);
    }
}

Element* findById(Node& node, std::string_view id) {
    for (const auto& child : node.children()) {
        if (child->type() != NodeType::Element) continue;
        auto* element = static_cast<Element*>(child.get());
        const auto* value = element->getAttribute("id");
        if (value != nullptr && *value == id) return element;
        if (auto* nested = findById(*element, id)) return nested;
    }
    return nullptr;
}

const Element* findById(const Node& node, std::string_view id) {
    for (const auto& child : node.children()) {
        if (child->type() != NodeType::Element) continue;
        const auto* element = static_cast<const Element*>(child.get());
        const auto* value = element->getAttribute("id");
        if (value != nullptr && *value == id) return element;
        if (const auto* nested = findById(*element, id)) return nested;
    }
    return nullptr;
}

} // namespace

Node::Node(NodeType type, std::string name, std::string data)
    : type_(type), name_(std::move(name)), data_(std::move(data)) {}

void Node::setData(std::string data) {
    if (type_ != NodeType::Text && type_ != NodeType::Comment) {
        throw std::logic_error("setData is only valid for text and comment nodes");
    }
    data_ = std::move(data);
}

Node* Node::firstChild() noexcept {
    return children_.empty() ? nullptr : children_.front().get();
}

const Node* Node::firstChild() const noexcept {
    return children_.empty() ? nullptr : children_.front().get();
}

Node& Node::appendChild(std::unique_ptr<Node> child) {
    if (!child) throw std::invalid_argument("Cannot append a null node");
    if (child.get() == this || child->parent_ != nullptr) {
        throw std::invalid_argument("Node is already attached or would create a cycle");
    }
    for (Node* ancestor = this; ancestor != nullptr; ancestor = ancestor->parent_) {
        if (ancestor == child.get()) {
            throw std::invalid_argument("Appending this node would create a cycle");
        }
    }
    if (type_ == NodeType::Text || type_ == NodeType::Comment) {
        throw std::invalid_argument("Text and comment nodes cannot have children");
    }
    if (type_ == NodeType::Document && child->type_ != NodeType::Element &&
        child->type_ != NodeType::Comment) {
        throw std::invalid_argument("A document can only contain elements and comments");
    }

    child->parent_ = this;
    children_.push_back(std::move(child));
    return *children_.back();
}

std::unique_ptr<Node> Node::removeChild(Node& child) {
    const auto it = std::find_if(children_.begin(), children_.end(),
        [&child](const std::unique_ptr<Node>& candidate) { return candidate.get() == &child; });
    if (it == children_.end()) throw std::invalid_argument("Node is not a direct child");
    std::unique_ptr<Node> removed = std::move(*it);
    children_.erase(it);
    removed->parent_ = nullptr;
    return removed;
}

std::string Node::textContent() const {
    if (type_ == NodeType::Text) return data_;
    if (type_ == NodeType::Comment) return {};
    std::string result;
    for (const auto& child : children_) result += child->textContent();
    return result;
}

std::string Node::serialize() const {
    if (type_ == NodeType::Text) return escapeText(data_);
    if (type_ == NodeType::Comment) return "<!--" + data_ + "-->";

    std::string result;
    for (const auto& child : children_) {
        if (child->type_ == NodeType::Element) {
            result += "<" + child->name_;
            const auto* element = static_cast<const Element*>(child.get());
            std::vector<std::pair<std::string, std::string>> attributes(
                element->attributes().begin(), element->attributes().end());
            std::sort(attributes.begin(), attributes.end(),
                [](const auto& left, const auto& right) { return left.first < right.first; });
            for (const auto& [key, value] : attributes) {
                result += " " + key + "=\"" + escapeAttribute(value) + "\"";
            }
            result += ">";
            if (!isVoidElement(child->name_)) {
                result += child->serialize();
                result += "</" + child->name_ + ">";
            }
        } else {
            result += child->serialize();
        }
    }
    return result;
}

Element::Element(std::string tagName)
    : Node(NodeType::Element, lowerAscii(std::move(tagName))) {
    if (name_.empty()) throw std::invalid_argument("Element tag name cannot be empty");
}

void Element::setTextContent(std::string value) {
    children_.clear();
    if (!value.empty()) appendChild(std::make_unique<Text>(std::move(value)));
}

void Element::setAttribute(std::string name, std::string value) {
    if (name.empty()) throw std::invalid_argument("Attribute name cannot be empty");
    attributes_[lowerAscii(std::move(name))] = std::move(value);
}

const std::string* Element::getAttribute(std::string_view name) const noexcept {
    const auto it = attributes_.find(lowerAscii(std::string(name)));
    return it == attributes_.end() ? nullptr : &it->second;
}

bool Element::hasAttribute(std::string_view name) const noexcept {
    return getAttribute(name) != nullptr;
}

bool Element::removeAttribute(std::string_view name) {
    return attributes_.erase(lowerAscii(std::string(name))) != 0;
}

Text::Text(std::string value) : Node(NodeType::Text, {}, std::move(value)) {}
Comment::Comment(std::string value) : Node(NodeType::Comment, {}, std::move(value)) {}
Document::Document() : Node(NodeType::Document) {}

std::unique_ptr<Element> Document::createElement(std::string tagName) const {
    return std::make_unique<Element>(std::move(tagName));
}
std::unique_ptr<Text> Document::createTextNode(std::string value) const {
    return std::make_unique<Text>(std::move(value));
}
std::unique_ptr<Comment> Document::createComment(std::string value) const {
    return std::make_unique<Comment>(std::move(value));
}

Element* Document::documentElement() noexcept {
    for (const auto& child : children_) {
        if (child->type() == NodeType::Element) return static_cast<Element*>(child.get());
    }
    return nullptr;
}
const Element* Document::documentElement() const noexcept {
    for (const auto& child : children_) {
        if (child->type() == NodeType::Element) return static_cast<const Element*>(child.get());
    }
    return nullptr;
}
Element* Document::getElementById(std::string_view id) noexcept { return findById(*this, id); }
const Element* Document::getElementById(std::string_view id) const noexcept {
    return findById(*this, id);
}
std::vector<Element*> Document::getElementsByTagName(std::string_view tagName) {
    std::vector<Element*> result;
    collectByTag(*this, lowerAscii(std::string(tagName)), result);
    return result;
}
std::vector<const Element*> Document::getElementsByTagName(std::string_view tagName) const {
    std::vector<const Element*> result;
    collectByTag(*this, lowerAscii(std::string(tagName)), result);
    return result;
}

} // namespace lapislazuli::dom
