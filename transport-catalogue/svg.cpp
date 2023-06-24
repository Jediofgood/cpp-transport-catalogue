#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        switch (line_join)
        {
        case svg::StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case svg::StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case svg::StrokeLineJoin::MITER:
            out << "miter";
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case svg::StrokeLineJoin::ROUND:
            out << "round";
            break;
        default:
            break;
        } return out;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    //PolyLine
    Polyline& Polyline::AddPoint(Point point) {
        container_of_points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool flag = true;
        for (const Point& p : container_of_points_) {
            if (flag) {
                flag = false;
            }
            else {
                out << " ";
            }
            out << p.x << "," << p.y;
        }
        out << "\"";
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    //text

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }

        out << ">"sv;

        for (const auto& c : data_) {
            if (c == '\"') {
                out << "&quot;"sv;
            }
            else if (c == '\'') {
                out << "&apos;"sv;
            }
            else if (c == '<') {
                out << "&lt;"sv;
            }
            else if (c == '>') {
                out << "&gt;"sv;
            }
            else if (c == '&') {
                out << "&amp;"sv;
            }
            else {
                out << c;
            }
        }
        out << "</text>"sv;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        RenderContext ctx(out, 2, 2);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& ptr : objects_) {
            ptr->Render(ctx);
        }
        out << "</svg>"sv;
    }

    //PrinterForColor
    void PrinterForColor::operator()(std::monostate) const {
        out << "none";
    }

    void PrinterForColor::operator()(const std::string& color) const {
        out << color;
    }

    void PrinterForColor::operator()(const Rgb& rgb) const {
        out << "rgb(" << static_cast<int>(rgb.red) << ","  //А почему не выводится нормально uint8_t?
            << static_cast<int>(rgb.green) << ","
            << static_cast<int>(rgb.blue) << ")";
    }

    void PrinterForColor::operator()(const Rgba& rgba) const {
        out << "rgba(" << static_cast<int>(rgba.red) << ","
            << static_cast<int>(rgba.green) << ","
            << static_cast<int>(rgba.blue) << ","
            << rgba.opacity << ")";
    }

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(PrinterForColor{ out }, color);
        return out;
    }

}  // namespace svg