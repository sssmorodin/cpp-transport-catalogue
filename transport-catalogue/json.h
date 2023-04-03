#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <algorithm>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

using Number = std::variant<int, double>;

class Node {
public:
   /* Реализуйте Node, используя std::variant */
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() = default;
    Node(std::nullptr_t) : value_(nullptr) { }
    Node(Array array) : value_(array) { }
    Node(Dict map) : value_(map) { }
    Node(bool val) : value_(val) { }
    Node(int val) : value_(val) { }
    Node(double val) : value_(val) { }
    Node(std::string value) : value_(value) { }


    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    const Value& GetValue() const { return value_; }
private:
    Value value_;
};

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);


// Шаблон, подходящий для вывода double и int
template <typename Value>
void PrintValue(const Value& value, std::ostream& out) {
    Node n(value);
    if (n.IsInt()) {
        out << n.AsInt();
    } else {
        out << n.AsDouble();
    }
}


// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out);
// Перегрузка функции PrintValue для вывода значений null
void PrintValue(bool value, std::ostream& out);
/*
void PrintValue(int value, std::ostream& out);
void PrintValue(double value, std::ostream& out);
*/
void PrintValue(std::string value, std::ostream& out);
void PrintNode(const Node& node, std::ostream& out);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    bool operator==(const Document& other);
    bool operator!=(const Document& other);

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json