#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace lapislazuli::dom {

enum class NodeType {
    Document,
    Element,
    Text,
    Comment
};

class Node {
public:
    explicit Node(NodeType type, std::string name = {}, std::string data = {});
    virtual ~Node() = default;

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    Node(Node&&) = delete;
    Node& operator=(Node&&) = delete;

    [[nodiscard]] NodeType type() const noexcept { return type_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& data() const noexcept { return data_; }
    void setData(std::string data);

    [[nodiscard]] Node* parent() noexcept { return parent_; }
    [[nodiscard]] const Node* parent() const noexcept { return parent_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Node>>& children() const noexcept {
        return children_;
    }
    [[nodiscard]] std::size_t childCount() const noexcept { return children_.size(); }
    [[nodiscard]] Node* firstChild() noexcept;
    [[nodiscard]] const Node* firstChild() const noexcept;

    // Transfers ownership to this node. Throws std::invalid_argument on invalid trees.
    Node& appendChild(std::unique_ptr<Node> child);
    // Removes a direct child and transfers ownership back to the caller.
    [[nodiscard]] std::unique_ptr<Node> removeChild(Node& child);

    [[nodiscard]] std::string textContent() const;
    [[nodiscard]] std::string serialize() const;

protected:
    NodeType type_;
    std::string name_;
    std::string data_;
    Node* parent_ = nullptr;
    std::vector<std::unique_ptr<Node>> children_;
};

class Element final : public Node {
public:
    explicit Element(std::string tagName);

    void setAttribute(std::string name, std::string value);
    [[nodiscard]] const std::string* getAttribute(std::string_view name) const;
    [[nodiscard]] bool hasAttribute(std::string_view name) const;
    [[nodiscard]] bool removeAttribute(std::string_view name);
    [[nodiscard]] const std::unordered_map<std::string, std::string>& attributes() const noexcept {
        return attributes_;
    }

private:
    std::unordered_map<std::string, std::string> attributes_;
};

class Text final : public Node {
public:
    explicit Text(std::string value);
};

class Comment final : public Node {
public:
    explicit Comment(std::string value);
};

class Document final : public Node {
public:
    Document();

    [[nodiscard]] std::unique_ptr<Element> createElement(std::string tagName) const;
    [[nodiscard]] std::unique_ptr<Text> createTextNode(std::string value) const;
    [[nodiscard]] std::unique_ptr<Comment> createComment(std::string value) const;

    [[nodiscard]] Element* documentElement() noexcept;
    [[nodiscard]] const Element* documentElement() const noexcept;
    [[nodiscard]] Element* getElementById(std::string_view id);
    [[nodiscard]] const Element* getElementById(std::string_view id) const;
    [[nodiscard]] std::vector<Element*> getElementsByTagName(std::string_view tagName);
    [[nodiscard]] std::vector<const Element*> getElementsByTagName(std::string_view tagName) const;
};

} // namespace lapislazuli::dom
