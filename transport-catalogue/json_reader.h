#pragma once

#include <sstream>

#include "json.h"
#include "transport_catalogue.h"

namespace jsonreader {

void LoadJson(std::istream& is, transport_catalogue::TrasportCatalogue* trc, json::Array* req_array, json::Dict* render_map);
	
void LoadFromJsonDB(const json::Array& base_array, transport_catalogue::TrasportCatalogue* trc);

void LoadStopsJson(const std::vector<const json::Node*>& stops, transport_catalogue::TrasportCatalogue* trc);

void LoadPrimaryStopInfo(const json::Node* stop_node, transport_catalogue::TrasportCatalogue* trc);

void LoadPrimaryBusInfo(const json::Node* bus_node, transport_catalogue::TrasportCatalogue* trc);

void LoadBusesJSON(const std::vector<const json::Node*>& buses, transport_catalogue::TrasportCatalogue* trc);
}

/*
void LoadBusInfo(const json::Node& bus_node, transport_catalogue::TrasportCatalogue* trc){
	const json::Dict& bus = bus_node.AsMap();
	const json::Array& stops_vector = bus.at("stops").AsArray();
	std::vector<std::string_view> stops_vec_sv;
	stops_vec_sv.reserve(stops_vec_sv.size());
	for (const json::Node stop : stops_vector) {
		stops_vec_sv.push_back(stop.AsString());
	}
	trc->AddBus(bus.at("name").AsString(), stops_vec_sv, bus.at("is_roundtrip").AsBool());
}

void LoadPrimaryStopInfo(const json::Node& stop_node, transport_catalogue::TrasportCatalogue* trc){
	json::Dict stop = stop_node.AsMap();
	trc->AddStop(stop.at("name").AsString(), geo::Coordinates{ stop.at("latitude").AsDouble(), stop.at("longitude").AsDouble() });
}

void LoadFromJsonDB(const json::Array& array_in, transport_catalogue::TrasportCatalogue* trc){
	std::vector<const json::Node&> stops_vec;
	std::vector<const json::Node&> bus_vec;

	for (const json::Node& data : array_in) {
		if (data.AsMap().at("type") == "stop") {
			stops_vec.push_back(&data);
		}
		else if (data.AsMap().at("type") == "bus") {
			bus_vec.push_back(&data);
		}
		else {
			throw std::invalid_argument("");
		}
	}

	std::unordered_map<std::string_view, const json::Node&> stops_leng;

	for (const json::Node& stop : stops_vec) {
		LoadPrimaryStopInfo(stop, trc);
		stops_leng.emplace(stop.AsMap().at("name").AsString(), stop);
	}

	for (const auto& [name, stop] : stops_leng) {
		for (const auto& other_stop : stop.AsMap().at("road_distances").AsArray()) {
		}
	}
}

void LoadJson(std::istream& is, transport_catalogue::TrasportCatalogue* trc){
	using namespace std::string_literals;
	json::Document doc = json::Load(is);
	const json::Dict& the_map = doc.GetRoot().AsMap();
	const json::Array& base_array = the_map.at("base_requests"s).AsArray();
	LoadFromJsonDB(base_array, trc);
}
void LoadStopFromJson(json::Node* node, transport_catalogue::TrasportCatalogue& trc) {
	//node - array of Dict
	node->AsArray();
}
*/
