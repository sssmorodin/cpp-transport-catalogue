#include "json_builder.h"

namespace json {

    KeyItemContext Builder::Key(std::string key) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Dictionary was not initialized");
        }
        if (Node{} == *nodes_stack_.back() &&
            !nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("No initialization for previous key value");
        }
        if (nodes_stack_.back()->IsDict()) {
            nodes_stack_.push_back(&std::get<Dict>(nodes_stack_.back()->GetNonConstValue())[key]);
        }
        if (nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Array declaration was not finished");
        }
        return KeyItemContext(*this);
    }

    Builder& Builder::Value(Node::Value value) {
        if (Node{} != root_ && nodes_stack_.empty()) {
            throw std::logic_error("Object is not ready");
        }
        if (nodes_stack_.empty()) {
            root_ = value;
            return *this;
        }
        if (nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Cannot be key for dict");
        }
        if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(nodes_stack_.back()->GetNonConstValue()).emplace_back(value);
        } else {
            *nodes_stack_.back() = value;
            nodes_stack_.pop_back();
        }
        return *this;
    }

    DictItemContext Builder::StartDict() {
        if (Node{} == root_) {
            root_ = Dict{};
            nodes_stack_.push_back(&root_);
            return DictItemContext(*this);
        } else if (nodes_stack_.empty()) {
            throw std::logic_error("Object is not ready");
        }
        if (nodes_stack_.back()->IsArray()) {
            auto& ref = std::get<Array>(nodes_stack_.back()->GetNonConstValue()).emplace_back(Dict{});
            nodes_stack_.push_back(&ref);
        } else if (Node{} == *nodes_stack_.back() &&
                   !nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsDict()) {
            *nodes_stack_.back() = Dict{};
        } else if (nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Dict cannot be key for another dict");
        }
        else {
            nodes_stack_.push_back(new Node(Dict{}));
        }
        return DictItemContext(*this);
    }

    ArrayItemContext Builder::StartArray() {
        if (Node{} == root_) {
            root_ = Array{};
            nodes_stack_.push_back(&root_);
            return ArrayItemContext(*this);
        } else if (nodes_stack_.empty()) {
            throw std::logic_error("Object is not ready");
        }
        if (nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Array cannot be key of dict");
        }
        if (Node{} == *nodes_stack_.back()) {
            *nodes_stack_.back() = Node(Array{});
        } else {
            nodes_stack_.push_back(new Node(Array{}));
        }
        return ArrayItemContext(*this);
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Object is not ready");
        }
        if (nodes_stack_.back()->IsDict()) {
            nodes_stack_.pop_back();
        } else {
            throw std::logic_error("Objects contexts conflict");
        }
        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Object is not ready");
        }
        if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.pop_back();
        } else {
            throw std::logic_error("Objects contexts conflict");
        }
        return *this;
    }

    Node Builder::Build() {
        if (Node{} == root_) {
            throw std::logic_error("Object is not ready");
        }
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Stack is not empty");
        }
        return root_;
    }

    //// Context classes

    Builder& BaseContext::GetBuilder() {
        return builder_;
    }

    // rule 3 context
    KeyItemContext DictItemContext::Key(std::string key) {
        return GetBuilder().Key(key);
    }
    Builder& DictItemContext::EndDict() {
        return GetBuilder().EndDict();
    }

    // rule 4 context
    ValueItemContextAfterArray ArrayItemContext::Value(Node::Value value) {
        return ValueItemContextAfterArray(GetBuilder().Value(value));
    }
    DictItemContext ArrayItemContext::StartDict() {
        return GetBuilder().StartDict();
    }
    ArrayItemContext ArrayItemContext::StartArray() {
        return GetBuilder().StartArray();
    }
    Builder& ArrayItemContext::EndArray() {
        return GetBuilder().EndArray();
    }

    // rule 1
    ValueItemContextAfterKey KeyItemContext::Value(Node::Value value) {
        return ValueItemContextAfterKey(GetBuilder().Value(value));
    }
    DictItemContext KeyItemContext::StartDict() {
        return GetBuilder().StartDict();
    }
    ArrayItemContext KeyItemContext::StartArray() {
        return GetBuilder().StartArray();
    }

    // rule 2
    KeyItemContext ValueItemContextAfterKey::Key(std::string key) {
        return GetBuilder().Key(key);
    }
    Builder& ValueItemContextAfterKey::EndDict() {
        return GetBuilder().EndDict();
    }

    // rule 5
    ValueItemContextAfterArray ValueItemContextAfterArray::Value(Node::Value value) {
        return ValueItemContextAfterArray(GetBuilder().Value(value));
    }
    DictItemContext ValueItemContextAfterArray::StartDict() {
        return GetBuilder().StartDict();
    }
    ArrayItemContext ValueItemContextAfterArray::StartArray() {
        return GetBuilder().StartArray();
    }
    Builder& ValueItemContextAfterArray::EndArray() {
        return GetBuilder().EndArray();
    }

} // namespace json
