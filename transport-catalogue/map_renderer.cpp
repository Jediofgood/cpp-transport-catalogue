#include "map_renderer.h"

#include <unordered_set>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <sstream>

#include "geo.h"
#include "transport_catalogue.h"

#include <svg.pb.h>
#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>

namespace render {

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

RenderSettings FillSettings(const json::Dict& render_map) {
    RenderSettings render_settings;
    render_settings.width = render_map.at("width").AsDouble();
    render_settings.height = render_map.at("height").AsDouble();
    render_settings.padding = render_map.at("padding").AsDouble();
    render_settings.line_width = render_map.at("line_width").AsDouble();
    render_settings.stop_radius = render_map.at("stop_radius").AsDouble();
    render_settings.bus_label_font_size = render_map.at("bus_label_font_size").AsInt();
    const json::Array& bus_label_offset = render_map.at("bus_label_offset").AsArray();
    render_settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
    render_settings.stop_label_font_size = render_map.at("stop_label_font_size").AsInt();
    const json::Array& stop_label_offset = render_map.at("stop_label_offset").AsArray();
    render_settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

    render_settings.underlayer_width = render_map.at("underlayer_width").AsDouble();

    if (render_map.at("underlayer_color").IsString()) {
        render_settings.underlayer_color = render_map.at("underlayer_color").AsString();
    }
    else {
        const json::Array& underlayer_color = render_map.at("underlayer_color").AsArray();
        if (underlayer_color.size() == 3) {
            render_settings.underlayer_color = svg::Rgb(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt());
        }
        else {
            render_settings.underlayer_color = svg::Rgba(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt(), underlayer_color[3].AsDouble());
        }
    }
    for(const auto& color_pallete_arr : render_map.at("color_palette").AsArray())
    {
        if (color_pallete_arr.IsString()) {
            render_settings.color_palette.push_back(color_pallete_arr.AsString());
        }
        else {
            const json::Array& color_palette = color_pallete_arr.AsArray();
            if (color_palette.size() == 3) {
                render_settings.color_palette.push_back(svg::Rgb(color_palette[0].AsInt(), color_palette[1].AsInt(), color_palette[2].AsInt()));
            }
            else {
                render_settings.color_palette.push_back(svg::Rgba(color_palette[0].AsInt(), color_palette[1].AsInt(), color_palette[2].AsInt(), color_palette[3].AsDouble()));
            }
        }
    }
    return render_settings;
}

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


//double max_width, double max_height, double padding
SphereProjector MakeSphereProjector(const RenderSettings& settings,
    const std::vector<geo::Coordinates>& stops
) {
    return SphereProjector{ stops.begin(), stops.end(), settings.width, settings.height, settings.padding };
}

void DrawBusRoutes(const std::map<std::string_view, transport_catalogue::Bus*>& bus_to_draw,
    svg::Document* doc_res, const SphereProjector& spp, 
    const RenderSettings& setting)
{
    int i = 0;
    unsigned size_color = setting.color_palette.size();

    for (const auto [namebus, busptr] : bus_to_draw) {
        svg::Polyline line;
        const std::vector<transport_catalogue::Stops*>& stops = busptr->GetRoute();
        for (auto stop : stops) {
            line.AddPoint(spp(stop->GetCoordinate()));
        }
        if (!busptr->IsRing()) {
            for (auto it = std::next(stops.rbegin()); it != stops.rend(); ++it) {
                line.AddPoint(spp((*it)->GetCoordinate()));
            }
        }
        line.SetStrokeColor(setting.color_palette[i++ % size_color]);
        line.SetFillColor("none");
        line.SetStrokeWidth(setting.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        auto rav = std::make_unique<svg::Polyline>(line);
        doc_res->AddPtr(std::move(rav));
    }
}

void VisualizeBusNameForRing(svg::Document* doc_res, const SphereProjector& spp,
    const RenderSettings& setting, transport_catalogue::Bus* busptr,
    int& i, int size_color) {

    const std::vector<transport_catalogue::Stops*>& stops = busptr->GetRoute();

    svg::Text text1;
    svg::Text under_text1;

    text1.SetPosition(spp(stops[0]->GetCoordinate()))
        .SetOffset(setting.bus_label_offset)
        .SetFontSize(setting.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(static_cast<std::string>(busptr->GetName()))
        .SetFillColor(setting.color_palette[i++ % size_color]);

    under_text1.SetPosition(spp(stops[0]->GetCoordinate()))
        .SetOffset(setting.bus_label_offset)
        .SetFontSize(setting.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(static_cast<std::string>(busptr->GetName()));

    under_text1.SetFillColor(setting.underlayer_color)
        .SetStrokeColor(setting.underlayer_color)
        .SetStrokeWidth(setting.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    auto underuniq_ptr = std::make_unique<svg::Text>(under_text1);
    doc_res->AddPtr(std::move(underuniq_ptr));

    auto uniq_ptr = std::make_unique<svg::Text>(text1);
    doc_res->AddPtr(std::move(uniq_ptr));
}

void VisualizeBusNameForLine(svg::Document* doc_res, const SphereProjector& spp,
    const RenderSettings& setting, transport_catalogue::Bus* busptr,
    int& i, int size_color) {

    const std::vector<transport_catalogue::Stops*>& stops = busptr->GetRoute();

    svg::Text text1;
    svg::Text under_text1;
    svg::Text text2;
    svg::Text under_text2;

    text1.SetPosition(spp(stops[0]->GetCoordinate()))
        .SetOffset(setting.bus_label_offset)
        .SetFontSize(setting.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(static_cast<std::string>(busptr->GetName()))
        .SetFillColor(setting.color_palette[i % size_color]);

    text2.SetPosition(spp(stops[stops.size() - 1]->GetCoordinate()))
        .SetOffset(setting.bus_label_offset)
        .SetFontSize(setting.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(static_cast<std::string>(busptr->GetName()))
        .SetFillColor(setting.color_palette[i++ % size_color]);

    under_text1.SetPosition(spp(stops[0]->GetCoordinate()))
        .SetOffset(setting.bus_label_offset)
        .SetFontSize(setting.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(static_cast<std::string>(busptr->GetName()));

    under_text1.SetFillColor(setting.underlayer_color)
        .SetStrokeColor(setting.underlayer_color)
        .SetStrokeWidth(setting.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    under_text2.SetPosition(spp(stops[stops.size() - 1]->GetCoordinate()))
        .SetOffset(setting.bus_label_offset)
        .SetFontSize(setting.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(static_cast<std::string>(busptr->GetName()));

    under_text2.SetFillColor(setting.underlayer_color)
        .SetStrokeColor(setting.underlayer_color)
        .SetStrokeWidth(setting.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    auto under_uniq_ptr1 = std::make_unique<svg::Text>(under_text1);
    auto under_uniq_ptr2 = std::make_unique<svg::Text>(under_text2);

    auto uniq_ptr1 = std::make_unique<svg::Text>(text1);
    auto uniq_ptr2 = std::make_unique<svg::Text>(text2);

    doc_res->AddPtr(std::move(under_uniq_ptr1));
    doc_res->AddPtr(std::move(uniq_ptr1));
    doc_res->AddPtr(std::move(under_uniq_ptr2));


    doc_res->AddPtr(std::move(uniq_ptr2));
}

void VisualizeBusName(const std::map<std::string_view, transport_catalogue::Bus*>& bus_to_draw,
    svg::Document* doc_res, const SphereProjector& spp,
    const RenderSettings& setting) {
    
    int i = 0;
    int size_color = setting.color_palette.size();

    svg::Text text;
    for (const auto [namebus, busptr] : bus_to_draw) {
        const std::vector<transport_catalogue::Stops*>& stops = busptr->GetRoute();

        if (busptr->IsRing() || stops[0] == stops[stops.size() - 1]) {//круговой случий
            VisualizeBusNameForRing(doc_res, spp, setting, busptr, i, size_color);
        } else{
            VisualizeBusNameForLine(doc_res, spp, setting, busptr, i, size_color);
        }

    }
}



void AddStopsPoint(std::map<std::string_view, const transport_catalogue::Stops*>& stop_to_draw,
    svg::Document* doc_res, const SphereProjector& spp,
    const RenderSettings& setting) {
    for (const auto[name, stop] : stop_to_draw) {
        svg::Circle point;
        point.SetCenter(spp(stop->GetCoordinate()))
            .SetRadius(setting.stop_radius)
            .SetFillColor("white");
        auto toadd = std::make_unique<svg::Circle>(point);
        doc_res->AddPtr(std::move(toadd));
    }
}

void AddStopsName(std::map<std::string_view, const transport_catalogue::Stops*>& stop_to_draw,
    svg::Document* doc_res, const SphereProjector& spp,
    const RenderSettings& setting) {
    for (const auto [name, stop] : stop_to_draw) {
        svg::Text stop_name;
        svg::Text under_text;

        stop_name.SetPosition(spp(stop->GetCoordinate()))
            .SetOffset(setting.stop_label_offset)
            .SetFontSize(setting.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(static_cast<std::string>(stop->GetName()));

        stop_name.SetFillColor("black");

        under_text.SetPosition(spp(stop->GetCoordinate()))
            .SetOffset(setting.stop_label_offset)
            .SetFontSize(setting.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(static_cast<std::string>(stop->GetName()));

        under_text.SetFillColor(setting.underlayer_color)
            .SetStrokeColor(setting.underlayer_color)
            .SetStrokeWidth(setting.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        auto under = std::make_unique<svg::Text>(under_text);
        auto text = std::make_unique<svg::Text>(stop_name);

        doc_res->AddPtr(std::move(under));
        doc_res->AddPtr(std::move(text));}
}


svg::Document MapMaker(
    RenderSettings setting,  //как её сделать
    const transport_catalogue::TransportCatalogue* trc) {

    std::vector<std::unique_ptr<svg::Object>> bus_route;

    svg::Document doc_res;

    std::map<std::string_view, transport_catalogue::Bus*> bus_to_draw;
    std::vector<geo::Coordinates> stops_geo;
    std::map<std::string_view, const transport_catalogue::Stops*> stops_render;
    for (const auto [bus_name, bus_ptr]:trc->GetBuses()) {
        if (bus_ptr->GetRoute().empty()) {
            continue;
        }
        else {
            bus_to_draw.emplace(bus_name, bus_ptr);
            for (auto stop : bus_ptr->GetRoute()) {
                stops_geo.push_back(stop->GetCoordinate());
                stops_render[stop->GetName()] = stop;
            }
        }
    }

    SphereProjector spp = MakeSphereProjector(setting, stops_geo);
    

    DrawBusRoutes(bus_to_draw, &doc_res, spp, setting);
    VisualizeBusName(bus_to_draw, &doc_res, spp, setting);
    AddStopsPoint(stops_render, &doc_res, spp, setting);
    AddStopsName(stops_render, &doc_res, spp, setting);
    


    return doc_res;
}

json::Node MapToNode(const svg::Document& svg_map, const json::Node& NodeId) {
    using namespace std::string_literals;
    json::Dict result;
    result["request_id"s] = NodeId.AsMap().at("id"s).AsInt();
    std::ostringstream output{};
    svg_map.Render(output);
    result["map"] = output.str();
    return json::Node{ result };
}

void FillProtoBuff(proto_render::RenderSettings* set, const json::Dict& render_map) {
    RenderSettings render_settings;
    
    set->set_width(render_map.at("width").AsDouble());
    set->set_height(render_map.at("height").AsDouble());
    set->set_padding(render_map.at("padding").AsDouble());
    set->set_line_width(render_map.at("line_width").AsDouble());
    set->set_stop_radius(render_map.at("stop_radius").AsDouble());
    set->set_bus_label_font_size(render_map.at("bus_label_font_size").AsInt());
    set->set_underlayer_width(render_map.at("underlayer_width").AsDouble());
    {
        proto_render::Point& point = *(set->mutable_bus_label_offset());
        const json::Array& bus_label_offset = render_map.at("bus_label_offset").AsArray();
        point.set_x(bus_label_offset[0].AsDouble());
        point.set_y(bus_label_offset[1].AsDouble());
    }

    set->set_stop_label_font_size(render_map.at("stop_label_font_size").AsInt());

    {
        proto_render::Point& point = *(set->mutable_stop_label_offset());
        const json::Array& stop_label_offset = render_map.at("stop_label_offset").AsArray();
        point.set_x(stop_label_offset[0].AsDouble());
        point.set_y(stop_label_offset[1].AsDouble());
    }
    set->set_underlayer_width(render_map.at("underlayer_width").AsDouble());

    proto_svg::Color& pb_underlayer_color = *set->mutable_underlayer_color();
    if (render_map.at("underlayer_color").IsString()) {
        pb_underlayer_color.set_str(render_map.at("underlayer_color").AsString());
    }
    else {
        const json::Array& underlayer_color = render_map.at("underlayer_color").AsArray();
        if (underlayer_color.size() == 3) {
            proto_svg::Rgb& rgb = *pb_underlayer_color.mutable_rgb();
            rgb.set_red(underlayer_color[0].AsInt());
            rgb.set_green(underlayer_color[1].AsInt());
            rgb.set_blue(underlayer_color[2].AsInt());
        }
        else if(underlayer_color.size() == 4) {
            proto_svg::Rgba& rgba = *pb_underlayer_color.mutable_rgba();
            rgba.set_red(underlayer_color[0].AsInt());
            rgba.set_green(underlayer_color[1].AsInt());
            rgba.set_blue(underlayer_color[2].AsInt());
            rgba.set_opacity(underlayer_color[3].AsDouble());
        }
        else {
            pb_underlayer_color.set_monostate(true);
        }
    }

    for (const json::Node& color_pallete_arr : render_map.at("color_palette").AsArray())
    {
        proto_svg::Color& color = *set->add_color_palette();

        if (color_pallete_arr.IsString()) {
            color.set_str(color_pallete_arr.AsString());
        }
        else {
            const json::Array& color_pallete = color_pallete_arr.AsArray();
            if (color_pallete.size() == 3) {
                proto_svg::Rgb& rgb = *color.mutable_rgb();
                rgb.set_red(color_pallete[0].AsInt());
                rgb.set_green(color_pallete[1].AsInt());
                rgb.set_blue(color_pallete[2].AsInt());
            }
            else if (color_pallete.size() == 4) {
                proto_svg::Rgba& rgba = *color.mutable_rgba();
                rgba.set_red(color_pallete[0].AsInt());
                rgba.set_green(color_pallete[1].AsInt());
                rgba.set_blue(color_pallete[2].AsInt());
                rgba.set_opacity(color_pallete[3].AsDouble());
            }
            else {
                color.set_monostate(false);
            }
        }
    }
}

RenderSettings FillSetFromProto(const proto_render::RenderSettings& pset) {
    RenderSettings render_settings;
    render_settings.width = pset.width();
    render_settings.height = pset.height();
    render_settings.padding = pset.padding();
    render_settings.line_width = pset.line_width();
    render_settings.stop_radius = pset.stop_radius();
    render_settings.bus_label_font_size = pset.bus_label_font_size();
    render_settings.bus_label_offset 
        = { pset.bus_label_offset().x(),pset.bus_label_offset().y() };
    render_settings.stop_label_font_size = pset.stop_label_font_size();
    render_settings.stop_label_offset 
        = { pset.stop_label_offset().x(), pset.stop_label_offset().y()};

    render_settings.underlayer_width = pset.underlayer_width();

    const proto_svg::Color& pb_underlayer_color = pset.underlayer_color();

    if (pb_underlayer_color.color_case() == 2) {
        render_settings.underlayer_color = pb_underlayer_color.str();
    }
    else {
        if (pb_underlayer_color.color_case() == 3) {
            const proto_svg::Rgb& rgb = pb_underlayer_color.rgb();
            render_settings.underlayer_color = 
                svg::Rgb(rgb.red(), rgb.green(), rgb.blue());
        }
        else if (pb_underlayer_color.color_case() == 4) {
            const proto_svg::Rgba& rgba = pb_underlayer_color.rgba();
            render_settings.underlayer_color = svg::Rgba(rgba.red(), rgba.green(), rgba.blue(), rgba.opacity());
        }
    }

    for (const proto_svg::Color& color_pallete : pset.color_palette())
    {
        if (color_pallete.color_case() == 2) {
            render_settings.color_palette.push_back(color_pallete.str());
        }
        else {
            if (color_pallete.color_case() == 3) {

                const proto_svg::Rgb& rgb = color_pallete.rgb();

                render_settings.color_palette
                    .push_back(svg::Rgb(rgb.red(), rgb.green(), rgb.blue()));

            }
            else if (color_pallete.color_case() == 4) {
                const proto_svg::Rgba& rgba = color_pallete.rgba();
                render_settings.color_palette
                    .push_back(svg::Rgba(rgba.red(), rgba.green(), rgba.blue(), rgba.opacity()));
            }
        }
    }

    return render_settings;
}

}//render