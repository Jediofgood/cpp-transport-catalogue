#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include <vector>
#include <deque>
#include <optional>
#include <variant>

namespace svg {
    struct Rgb {
        Rgb() = default;

        Rgb(uint8_t red_one, uint8_t green_one, uint8_t blue_one)
            :red(red_one), green(green_one), blue(blue_one)
        {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba
    {
        Rgba() = default;

        Rgba(uint8_t red_one, uint8_t green_one, uint8_t blue_one, double the_opacity)
            :red(red_one), green(green_one), blue(blue_one), opacity(the_opacity)
        {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{ std::monostate() };

    struct PrinterForColor
    {
        std::ostream& out;

        explicit PrinterForColor(std::ostream& os)
            :out(os)
        {}

        void operator()(std::monostate) const;

        void operator()(const std::string& color) const;

        void operator()(const Rgb& rgb) const;

        void operator()(const Rgba& rgba) const;
    };

    std::ostream& operator<<(std::ostream& out, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
     * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
     */             
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * ����������� ������� ����� Object ������ ��� ���������������� ��������
     * ���������� ����� SVG-���������
     * ��������� ������� "��������� �����" ��� ������ ����������� ����
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        template<typename T>
        void Add(T obj) {
            AddPtr(std::make_unique<T>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& oc) const = 0;

        virtual	~Drawable() = default;

    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);



    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast ��������� ����������� *this � Owner&,
            // ���� ����� Owner � ��������� PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;

        //std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
        //std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);
    };

    /*
     * ����� Circle ���������� ������� <circle> ��� ����������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // ��������� ��������� ������� � ������� �����
        Polyline& AddPoint(Point point);

        /*
         * ������ ������ � ������, ����������� ��� ���������� �������� <polyline>
         */
    private:
        //std::deque<Point> container_of_points_;
        std::vector<Point> container_of_points_;


        void RenderObject(const RenderContext& context) const override;
    };

    /*
     * ����� Text ���������� ������� <text> ��� ����������� ������
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text : public Object, public PathProps<Text> {
    public:
        // ����� ���������� ������� ����� (�������� x � y)
        Text& SetPosition(Point pos);

        // ����� �������� ������������ ������� ����� (�������� dx, dy)
        Text& SetOffset(Point offset);

        // ����� ������� ������ (������� font-size)
        Text& SetFontSize(uint32_t size);

        // ����� �������� ������ (������� font-family)
        Text& SetFontFamily(std::string font_family);

        // ����� ������� ������ (������� font-weight)
        Text& SetFontWeight(std::string font_weight);

        // ����� ��������� ���������� ������� (������������ ������ ���� text)
        Text& SetData(std::string data);

        // ������ ������ � ������, ����������� ��� ���������� �������� <text>
    private:
        Point pos_ = Point{ 0.0, 0.0 };
        Point offset_ = Point{ 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;

        void RenderObject(const RenderContext& context) const override;
    };

    class Document : public ObjectContainer {
    public:
        /*
         ����� Add ��������� � svg-�������� ����� ������-��������� svg::Object.
         ������ �������������:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
        */
        // void Add(???);
        template<typename Obj>
        void Add(Obj obj);

        // ��������� � svg-�������� ������-��������� svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj);

        // ������� � ostream svg-������������� ���������
        void Render(std::ostream& out) const;

        // ������ ������ � ������, ����������� ��� ���������� ������ Document
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    template<typename Obj>
    void Document::Add(Obj obj) {
        objects_.push_back(std::make_unique<Obj>(std::move(obj)));
    }

}  // namespace svg