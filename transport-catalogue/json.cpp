#include "json.h"
#include <execution>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);
        Node LoadArray(istream& input) {
            Array result;
            char c;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') {
                throw ParsingError("");
            }
            return Node(move(result));
        }

        Node LoadString(std::istream& input) {
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
                }
                else if (ch == '\\') {
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
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            return Node(std::move(s));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ std::move(key), std::move(LoadNode(input)) });
            }
            if (c != '}') {
                throw ParsingError("");
            }
            return Node(move(result));
        }

        Node LoadBool(std::istream& input, char bool_c) {
            if (bool_c == 'f') {
                std::string s = "f";
                char c;
                for (int i = 0; i < 4 and input.get(c); ++i) {
                    s += c;
                }
                if ("false" == s) {
                    return Node{ false };
                }
                else {
                    throw ParsingError("");
                }
            }
            if (bool_c == 't') {
                std::string s = "t";
                char c;
                for (int i = 0; i < 3 and input.get(c); ++i) {
                    s += c;
                }
                if (s == "true") {
                    return Node{ true };
                }
                else {
                    throw ParsingError("");
                }
            }
            return Node{ true };
        }

        Node LoadNull(std::istream& input) {
            std::string s = "n";
            char c;
            for (int i = 0; i < 3 and input.get(c); ++i) {
                s += c;
            }
            if (s == "null") {
                return Node{ nullptr };
            }
            else {
                throw ParsingError("");
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;
            switch (c) {
            case '[':
                return LoadArray(input);
                break;
            case '{':
                return LoadDict(input);
                break;
            case '"':
                return LoadString(input);
                break;
            case 'n':
                //return Node{};
                return LoadNull(input);
                break;
            case't':
                //return Node{ true };
                return LoadBool(input, c);
                break;
            case 'f':
                //return Node{ false };
                return LoadBool(input, c);
                break;
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }


    }  // namespace

    Node::Node(std::nullptr_t)
        :value_(nullptr)
    {}

    Node::Node(Array array)
        : value_(move(array)) {
    }

    Node::Node(Dict map)
        : value_(move(map)) {
    }

    Node::Node(int value)
        : value_(value) {
    }

    Node::Node(double value)
        : value_(value) {
    }

    Node::Node(string value)
        : value_(move(value)) {
    }

    Node::Node(bool value)
        : value_(move(value)) {
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("");
        }
        return std::get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("");
        }
        return std::get<Dict>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("");
        }
        if (IsInt()) {
            return static_cast<double>(std::get<int>(value_));
        }
        return std::get<double>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("");
        }
        return std::get<int>(value_);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("");
        }
        return std::get<std::string>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("");
        }
        return std::get<bool>(value_);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        std::visit(json::TheNodePrinter{ output }, doc.GetRoot().GetValue());
    }
    
    void Print1(const Document& doc, std::ostream& output) {
        PrintContext crez(output);

        json::TheNodePrinterContext printer(crez);

        std::visit(json::TheNodePrinterContext{ printer }, doc.GetRoot().GetValue());
    }

    /*
    ValueType IsValue::operator()(std::nullptr_t) const {
        return ValueType::THENULL;
    }
    ValueType IsValue::operator()(const Array&) const {
    ValueType IsValue::operator()(const Array&) const {
        return ValueType::ARRAY;
    }
    ValueType IsValue::operator()(const Dict&) const {
        return ValueType::MAP;
    }
    ValueType IsValue::operator()(bool) const {
        return ValueType::BOOL;
    }
    ValueType IsValue::operator()(int) const {
        return ValueType::INT;
    }
    ValueType IsValue::operator()(double) const {
        return ValueType::DOUBLE;
    }
    ValueType IsValue::operator()(const std::string&) const {
        return ValueType::STRING;
    }
    */

    //AsValue::


    //json



    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const {
        return (std::holds_alternative<double>(value_) or std::holds_alternative<int>(value_));
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }
    bool Node::IsString() const {

        return std::holds_alternative<std::string>(value_);
    }
    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const {

        return std::holds_alternative<Dict>(value_);
    }

    bool Node::operator==(const Node& rhs) {
        return GetValue() == rhs.GetValue();
    }

    bool Node::operator!=(const Node& rhs) {
        return GetValue() != rhs.GetValue();
    }

    const Node::Value& Node::GetValue() const {
        return value_;
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }
    bool operator!=(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() != rhs.GetValue();
    }

    //TheNodePrinter::
    void TheNodePrinter::operator()(std::nullptr_t) const {
        out << "null";
    }
    void TheNodePrinter::operator()(const Array& array) const {
        out << "[ ";
        bool flag = true;
        for (const Node& elem : array) {
            if (flag) {
                flag = !flag;
                std::visit(*this, elem.GetValue());
            }
            else
            {
                out << ", ";
                std::visit(*this, elem.GetValue());
                out << " ";
            }
        }
        out << "]";
    }
    void TheNodePrinter::operator()(const Dict& dict) const {
        out << "{ ";
        bool flag = true;
        for (const auto& [str, elem] : dict) {
            if (flag) {
                flag = !flag;
                out << "\"" << str << "\": ";
                std::visit(*this, elem.GetValue());
            }
            else
            {
                out << ", ";
                out << "\"" << str << "\": ";
                std::visit(*this, elem.GetValue());
                out << " ";
            }
        }
        out << "}";
    }
    void TheNodePrinter::operator()(const bool elem) const {
        if (elem) {
            out << "true";
        }
        else {
            out << "false";
        }
    }
    void TheNodePrinter::operator()(const int elem) const {
        out << elem;
    }
    void TheNodePrinter::operator()(const double elem) const {
        out << elem;
    }
    void TheNodePrinter::operator()(const std::string& elem) const {
        out << "\"";
        UpLoadString(out, elem);
        out << "\"";
    }

    Node LoadNumber(std::istream& input) {
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
        }
        else {
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
                    return Node{ std::stoi(parsed_num) };
                }
                catch (...) {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return Node{ std::stod(parsed_num) };
        }
        catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    void UpLoadString(std::ostream& out, const std::string& str) {
        using namespace std::literals;
        //последовательности: \n, \r, \", \t, \\.
        for (const char& c : str) {
            switch (c) {
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\\':
                out << "\\\\";
                break;
            case '\"':
                out << "\\\"";
                break;
            default:
                out << c;
                break;
            }
        }
    }

    bool operator==(Document left, Document right) {
        return left.GetRoot() == right.GetRoot();
    }

    bool operator!=(Document left, Document right) {
        return left.GetRoot() != right.GetRoot();
    }

    



    //TheNodePrinterContext::
    void TheNodePrinterContext::operator()(std::nullptr_t) const {
        //out << "null";
    }
    void TheNodePrinterContext::operator()(const Array& array) const {
        //out << "[ ";
        con.out << "[\n ";
        bool flag = true;
        PrintContext other_cont = con.Indented();
        for (const Node& elem : array) {
            if (flag) {
                flag = !flag;
                other_cont.PrintIndent();
                //std::visit(*this, elem.GetValue());
                std::visit(TheNodePrinterContext{ other_cont }, elem.GetValue());
            }
            else
            {
                con.out << ", ";
                //std::visit(*this, elem.GetValue());
                std::visit(TheNodePrinterContext{ other_cont }, elem.GetValue());
            }
        }
        con.out << "\n";
        con.PrintIndent();
        con.out << "]";
    }
    void TheNodePrinterContext::operator()(const Dict& dict) const {
        con.out << "\n";
        con.PrintIndent();
        con.out << "{\n";
        bool flag = true;
        PrintContext other_con = con.Indented();
        for (const auto& [str, elem] : dict) {
            if (flag) {
                flag = !flag;
                other_con.PrintIndent();
                con.out << "\"" << str << "\": ";
                std::visit(*this, elem.GetValue());
                //std::visit(TheNodePrinterContext{con.Indented()}, elem.GetValue());
            }
            else
            {
                
                con.out << ", ";
                con.out << "\n";
                other_con.PrintIndent();
                con.out << "\"" << str << "\": ";
                std::visit(*this, elem.GetValue());
                con.out << " ";
            }
        }
        con.out << "\n";
        con.PrintIndent();
        con.out << "}";
    }
    void TheNodePrinterContext::operator()(const bool elem) const {
        if (elem) {
            con.out << "true";
        }
        else {
            con.out << "false";
        }
    }
    void TheNodePrinterContext::operator()(const int elem) const {
        con.out << elem;
    }
    void TheNodePrinterContext::operator()(const double elem) const {
        con.out << elem;
    }
    void TheNodePrinterContext::operator()(const std::string& elem) const {
        con.out << "\"";
        UpLoadString(con.out, elem);
        con.out << "\"";
    }


}  // namespace json