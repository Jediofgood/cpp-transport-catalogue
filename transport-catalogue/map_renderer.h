#pragma once

#include <vector>

#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"

namespace render{

bool IsZero(double value);

struct RenderSettings{
public:
	double width = 0.0;
	double height = 0.0;
	double padding = 0.0;
	double line_width = 0.0;
	double stop_radius = 0.0;
	int bus_label_font_size = 0;
	svg::Point bus_label_offset = { 0.0, 0.0 };

	int stop_label_font_size = 0.;
	svg::Point stop_label_offset = { 0.0, 0.0 };
	svg::Color underlayer_color{ svg::NoneColor };
	double underlayer_width = 0.;
	std::vector<svg::Color> color_palette{};
};

RenderSettings FillSettings(const json::Dict& render_map);

class SphereProjector;

void DrawBusRoutes(const std::map<std::string_view, transport_catalogue::Bus*>& bus_to_draw,
	svg::Document* doc_res, const SphereProjector& spp,
	const RenderSettings& setting);

void AddStopsPoint(const std::map<std::string_view, const transport_catalogue::Stops*>& stop_to_draw,
	svg::Document* doc_res, const SphereProjector& spp,
	const RenderSettings& setting);

void AddStopsName(std::map<std::string_view, const transport_catalogue::Stops*>& stop_to_draw,
	svg::Document* doc_res, const SphereProjector& spp,
	const RenderSettings& setting);

svg::Document MapMaker(
	const json::Dict& render_map,
	const transport_catalogue::TrasportCatalogue* trc);

}//render
