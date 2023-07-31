#include "json_reader.h"
#include "geo.h"
#include "request_handler.h"
#include "map_renderer.h"


#include <string>
#include <unordered_map>
#include <execution>

namespace jsonreader {
	
void LoadJson(std::istream& is, 
		transport_catalogue::TrasportCatalogue* trc, 
		json::Array* req_array, 
		json::Dict* render_map)
{
	json::Document doc = json::Load(is);
	const json::Dict& the_map = doc.GetRoot().AsMap();
	const json::Array& base_array = the_map.at("base_requests").AsArray();
	*req_array = the_map.at("stat_requests").AsArray();
	if(the_map.count("render_settings")){
		*render_map = the_map.at("render_settings").AsMap();
	}
	LoadFromJsonDB(base_array, trc);
}

void LoadJson(std::istream& is,
		transport_catalogue::TrasportCatalogue* trc,
		json::Node& tr_set,
		json::Array* req_array,
		json::Dict* render_map)
{
	json::Document doc = json::Load(is);
	const json::Dict& the_map = doc.GetRoot().AsMap();
	const json::Array& base_array = the_map.at("base_requests").AsArray();
	*req_array = the_map.at("stat_requests").AsArray();
	if (the_map.count("render_settings")) {
		*render_map = the_map.at("render_settings").AsMap();
	}
	tr_set = (the_map.at("routing_settings"));
	LoadFromJsonDB(base_array, trc);
}

void LoadFromJsonDB(const json::Array& base_array, transport_catalogue::TrasportCatalogue* trc) {
	using namespace json;
	//Вектор остановок и автобусов

	std::vector<const Node*> stops;
	std::vector<const Node*> buses;

	for (const Node& node : base_array) {
		if (node.AsMap().at("type").AsString() == "Stop") {
			stops.push_back(&node);
		}
		else if (node.AsMap().at("type").AsString() == "Bus") {
			buses.push_back(&node);
		}
		else {
			throw std::invalid_argument("");
		}
	}

	LoadStopsJson(stops, trc);
	LoadBusesJSON(buses, trc);
}

void LoadPrimaryBusInfo(const json::Node* bus_node, transport_catalogue::TrasportCatalogue* trc) {
	const json::Dict& bus_map = bus_node->AsMap();
	std::vector<std::string_view> stops;
	for (const json::Node& stops_node : bus_map.at("stops").AsArray()) {
		stops.push_back(stops_node.AsString());
	}
	trc->AddBus(
		bus_map.at("name").AsString(),
		stops,
		bus_map.at("is_roundtrip").AsBool()
	);
}

void LoadBusesJSON(const std::vector<const json::Node*>& buses, transport_catalogue::TrasportCatalogue* trc) {
	for (const json::Node* bus : buses) {
		LoadPrimaryBusInfo(bus, trc);
	}
}

void LoadPrimaryStopInfo(const json::Node* stop_node, transport_catalogue::TrasportCatalogue* trc) {
	json::Dict stop = stop_node->AsMap();
	trc->AddStop(stop.at("name").AsString(), geo::Coordinates{ stop.at("latitude").AsDouble(), stop.at("longitude").AsDouble() });
}

void LoadStopsJson(const std::vector<const json::Node*>& stops, transport_catalogue::TrasportCatalogue* trc) {

	std::unordered_map<std::string_view, const json::Node*> stops_leng;
	for (const json::Node* stop : stops) {
		LoadPrimaryStopInfo(stop, trc);
		//stops_leng.emplace(stop->AsMap().at("name").AsString(), stop->AsMap().at("road_distances"));
		stops_leng[stop->AsMap().at("name").AsString()] = &(stop->AsMap().at("road_distances"));
	}

	for (const auto [currect_name, to_stops_map] : stops_leng) {
		for (const auto& [stop_name, leng_node] : to_stops_map->AsMap()) {
			trc->AddLenghtBetweenStops(currect_name, stop_name, leng_node.AsDouble());
		}
	}
}

}