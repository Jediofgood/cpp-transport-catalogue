#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <variant>

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

    class Node {
    public:
        /* Реализуйте Node, используя std::variant */
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;

        Node(std::nullptr_t);
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(double value);
        Node(std::string value);
        Node(bool value);

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        bool AsBool() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const Value& GetValue() const;

        bool operator==(const Node& rhs);
        bool operator!=(const Node& rhs);

    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;
    private:
        Node root_;
    };

    Document Load(std::istream& input);



    void Print(const Document& doc, std::ostream& output);
    void Print1(const Document& doc, std::ostream& output);

    bool operator==(const Node& lhs, const Node& rhs);
    bool operator!=(const Node& lhs, const Node& rhs);

    class TheNodePrinter {
    public:
        std::ostream& out;

        explicit TheNodePrinter(std::ostream& os)
            :out(os)
        {}

        void operator()(std::nullptr_t) const;
        void operator()(const Array&) const;
        void operator()(const Dict&) const;
        void operator()(const bool) const;
        void operator()(const int) const;
        void operator()(const double) const;
        void operator()(const std::string&) const;
    };

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        PrintContext(std::ostream& out1) :out(out1) {}
        PrintContext(std::ostream& out1, int indent_step1, int indent1)
            :out(out1), indent_step(indent_step1), indent(indent1)
        {}

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }
        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    class TheNodePrinterContext {
    public:
        const PrintContext& con;

        explicit TheNodePrinterContext(const PrintContext& cont)
            :con(cont)
        {}

        void operator()(std::nullptr_t) const;
        void operator()(const Array&) const;
        void operator()(const Dict&) const;
        void operator()(const bool) const;
        void operator()(const int) const;
        void operator()(const double) const;
        void operator()(const std::string&) const;
    };

    Node LoadNumber(std::istream& input);
    void UpLoadString(std::ostream& out, const std::string& str);
    bool operator==(Document left, Document right);
    bool operator!=(Document left, Document right);

}  // namespace json