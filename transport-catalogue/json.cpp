#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            if (it == end) {
                // Поток закончился до первого символа
                throw ParsingError("String parsing error");
            }
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadDict(istream& input) {
            Dict result;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            if (it == end) {
                // Поток закончился до первого символа
                throw ParsingError("String parsing error");
            }
            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }

            return Node(move(result));
        }

        Node LoadBool(istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while ("true"s != s && s.size() < 5 ) {
                if (it == end) {
                    // Поток закончился до того, как собралось true
                    throw ParsingError("String parsing error");
                }
                s.push_back(*it);
                ++it;
            }
            return "false"s == s ? Node(false) : Node(true);
        }

        Node LoadNull(istream& input) {
            //
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while ("null"s != s && s.size() < 4 ) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                s.push_back(*it);
                ++it;
            }
            return Node(nullptr);
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return Node(LoadString(input));
            } else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            } else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            } else {
                input.putback(c);
                auto number = LoadNumber(input);
                if (holds_alternative<double>(number)) {
                    return Node(get<double>(number));
                }
                return Node(get<int>(number));
            }
        }

    }  // namespace

    bool Node::IsInt() const {
        return holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const {
        return holds_alternative<double>(value_) || holds_alternative<int>(value_);
    }
    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value_);
    }
    bool Node::IsBool() const {
        return holds_alternative<bool>(value_);
    }
    bool Node::IsString() const {
        return holds_alternative<std::string>(value_);
    }
    bool Node::IsNull() const {
        return holds_alternative<std::nullptr_t>(value_);
    }
    bool Node::IsArray() const {
        return holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const {
        return holds_alternative<Dict>(value_);
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        if (lhs.IsBool() && rhs.IsBool()) {
            return lhs.AsBool() == rhs.AsBool();
        }
        if (lhs.IsInt() && rhs.IsInt()) {
            return lhs.AsInt() == rhs.AsInt();
        }
        if (lhs.IsPureDouble() && rhs.IsPureDouble()) {
            return lhs.AsDouble() == rhs.AsDouble();
        }
        if (lhs.IsNull() && rhs.IsNull()) {
            return true;
        }
        if (lhs.IsString() && rhs.IsString()) {
            return lhs.AsString() == rhs.AsString();
        }
        if (lhs.IsMap() && rhs.IsMap()) {
            return lhs.AsMap() == rhs.AsMap();
        }
        if (lhs.IsArray() && rhs.IsArray()) {
            return lhs.AsArray() == rhs.AsArray();
        } return false;
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return get<Array>(value_);
        }
        throw std::logic_error("");
    }
    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return get<Dict>(value_);
        }
        throw std::logic_error("");
    }
    int Node::AsInt() const {
        if (IsInt()) {
            return get<int>(value_);
        }
        throw std::logic_error("");
    }
    const string& Node::AsString() const {
        if (IsString()) {
            return get<string>(value_);
        }
        throw std::logic_error("");
    }
    bool Node::AsBool() const {
        if (IsBool()) {
            return get<bool>(value_);
        }
        throw std::logic_error("");
    }
    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return get<double>(value_);
        } else if (IsInt()) {
            return static_cast<double>(get<int>(value_));
        }
        throw std::logic_error("");
    }


// Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, std::ostream& out) {
        using namespace std::string_view_literals;
        out << "null"sv;
    }
// Перегрузка функции PrintValue для вывода значений bool
    void PrintValue(bool value, std::ostream& out) {
        out << boolalpha << value;
    }

    void PrintValue(Array array, std::ostream& out) {
        out << '[';
        bool is_first = true;
        for (auto& value : array) {
            if (!is_first) {
                out << ',';
            }
            is_first = false;
            PrintNode(value, out);
        }
        out << ']';
    }

    void PrintValue(Dict dict, std::ostream& out) {
        out << '{' << ' ';
        bool is_first = true;
        for (auto [key, value] : dict) {
            if (!is_first) {
                out << ',' << ' ';
            }
            is_first = false;
            out << '\"' << key << "\": "s;
            PrintNode(value, out);
            //out << '\"';
        }
        out << '}';
    }

    void PrintValue(std::string value, std::ostream& out) {
        vector<char> escape_chars{'\\', '\"', '\n', '\r', '\t',};
        out << '\"';
        for (const char& c : value) {
            switch (c) {
                case '\r':
                    out << "\\r"sv;
                    break;
                case '\n':
                    out << "\\n"sv;
                    break;
                case '\\':
                    out << "\\\\"sv;
                    break;
                case '\"':
                    out << "\\\""sv;
                    break;
                default:
                    out << c;
            }
        }
        out << '\"';
    }
    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
                [&out](const auto& value){ PrintValue(value, out); },
                node.GetValue());
    }

    Document::Document(Node root)
            : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void Print(const Document& doc, std::ostream& output)  {
        const auto nodes = doc.GetRoot();
        const auto nodes_v = doc.GetRoot().GetValue();
        PrintNode(nodes, output);
    }

    bool Document::operator==(const Document& other) {
        return GetRoot() == other.GetRoot();
    }
    bool Document::operator!=(const Document& other) {
        return !(GetRoot() == other.GetRoot());
    }
}  // namespace json