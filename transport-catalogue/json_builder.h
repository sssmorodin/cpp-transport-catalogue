#pragma once

#include <vector>
#include <memory>
#include <variant>

#include "json.h"

namespace json {

    class BaseContext;
    class DictItemContext;
    class KeyItemContext;
    class ArrayItemContext;
    class ValueItemContextAfterKey;
    class ValueItemContextAfterArray;

    class Builder {
    public:
        Builder() = default;
        KeyItemContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
    };

    //// Context classes

    /* Сompilation prohibition rules :
       1 Непосредственно после Key вызван не Value, не StartDict и не StartArray.
       2 После вызова Value, последовавшего за вызовом Key, вызван не Key и не EndDict.
       3 За вызовом StartDict следует не Key и не EndDict.
       4 За вызовом StartArray следует не Value, не StartDict, не StartArray и не EndArray.
       5 После вызова StartArray и серии Value следует не Value, не StartDict, не StartArray и не EndArray.
     */

    class BaseContext {
    public:
        explicit BaseContext(Builder& builder)
            : builder_(builder) {
        }
        Builder& GetBuilder();
    private:
        Builder& builder_;
    };

    // rule 3 context
    class DictItemContext : public BaseContext {
    public:
        explicit DictItemContext(Builder& builder)
            : BaseContext(builder) {
        }
        KeyItemContext Key(std::string key);
        Builder& EndDict();
    };

    // rule 1
    class KeyItemContext : public BaseContext {
    public:
        explicit KeyItemContext(Builder& builder)
            : BaseContext(builder) {
        }
        ValueItemContextAfterKey Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    };

    // rule 4 context
    class ArrayItemContext : public BaseContext {
    public:
        explicit ArrayItemContext(Builder& builder)
            : BaseContext(builder) {
        }
        ValueItemContextAfterArray Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
    };

    // rule 2
    class ValueItemContextAfterKey : public BaseContext {
    public:
        explicit ValueItemContextAfterKey(Builder& builder)
            : BaseContext(builder) {
        }
        KeyItemContext Key(std::string key);
        Builder& EndDict();
    };

    // rule 5
    class ValueItemContextAfterArray : public BaseContext {
    public:
        explicit ValueItemContextAfterArray(Builder& builder)
        : BaseContext(builder) {
        }
        ValueItemContextAfterArray Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
    };

} // namespace json

