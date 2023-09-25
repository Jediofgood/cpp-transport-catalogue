#include "json_reader.h"
#include "geo.h"
#include "request_handler.h"
#include "map_renderer.h"


#include <string>
#include <unordered_map>
#include <set>

namespace jsonreader {
	
void LoadJson(std::istream& is, 
		transport_catalogue::TransportCatalogue* trc, 
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
		transport_catalogue::TransportCatalogue* trc,
		json::Node& tr_set,
		json::Array* req_array,
		json::Dict* render_map
	)
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

void LoadFromJsonDB(const json::Array& base_array, transport_catalogue::TransportCatalogue* trc) {
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

void LoadPrimaryBusInfo(const json::Node* bus_node, transport_catalogue::TransportCatalogue* trc) {
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

void LoadBusesJSON(const std::vector<const json::Node*>& buses, transport_catalogue::TransportCatalogue* trc) {
	for (const json::Node* bus : buses) {
		LoadPrimaryBusInfo(bus, trc);
	}
}

void LoadPrimaryStopInfo(const json::Node* stop_node, transport_catalogue::TransportCatalogue* trc) {
	json::Dict stop = stop_node->AsMap();
	trc->AddStop(stop.at("name").AsString(), geo::Coordinates{ stop.at("latitude").AsDouble(), stop.at("longitude").AsDouble() });
}

void LoadStopsJson(const std::vector<const json::Node*>& stops, transport_catalogue::TransportCatalogue* trc) {

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

//protobuff start here

void MakeBaseJson(std::istream& is,
		json::Array* base_array,
		json::Node* tr_set,
		json::Dict* render_map,
		json::Dict* serial_settings)
{
	json::Document doc = json::Load(is);
	const json::Dict& the_map = doc.GetRoot().AsMap();

	*base_array = the_map.at("base_requests").AsArray();
	if (the_map.count("routing_settings")) {
		*tr_set = the_map.at("routing_settings");
	}
	if (the_map.count("render_settings")) {
* render_map = the_map.at("render_settings").AsMap();
	}
	if (the_map.count("serialization_settings")) {
		*serial_settings = the_map.at("serialization_settings").AsMap();
	}
}

void SortBaseRequest(const json::Array& base_array, RequestVectors* vecs) {
	using namespace json;
	//Вектор остановок и автобусов

	//std::vector<const Node*> stops;
	//std::vector<const Node*> buses;

	for (const Node& node : base_array) {
		if (node.AsMap().at("type").AsString() == "Stop") {
			vecs->stops.push_back(&node);
		}
		else if (node.AsMap().at("type").AsString() == "Bus") {
			vecs->buses.push_back(&node);
		}
		else {
			throw std::invalid_argument("");
		}
	}
}

//std::unordered_map<std::string_view, size_t> LoadProtoStopsJson(const std::vector<const json::Node*>& stops, transport_catalogue_proto::TransportProto* database) {
void LoadProtoStopsJson(const std::vector<const json::Node*>& stops,
	transport_catalogue_proto::TransportProto* database,
	std::unordered_map<std::string_view, size_t>* ptr_assigned_id
) {

	std::unordered_map<std::string_view, const json::Node*> stops_leng;

	std::unordered_map<size_t, const json::Dict*> id_stops;
	std::unordered_map<std::string_view, size_t>& assigned_id = *ptr_assigned_id;

	size_t id = 0;

	for (const json::Node* stop_node : stops) {

		//void/transport_catalogue_proto::Stops* SetPBStops (const json::Node* stop_node, size_t id /*, transport_catalogue_proto::TransportProto* database*/) {
		transport_catalogue_proto::Stops* pb_stop = database->add_stops();

		const json::Dict& stop_dict = stop_node->AsMap();

		pb_stop->set_id(id++);
		pb_stop->set_stop_name(stop_dict.at("name").AsString());


		pb_stop->mutable_coordinates()->set_lat(stop_dict.at("latitude").AsDouble());
		pb_stop->mutable_coordinates()->set_lon(stop_dict.at("longitude").AsDouble());


		id_stops[pb_stop->id()] = &stop_dict;
		assigned_id[pb_stop->stop_name()] = pb_stop->id();
	}

	for (const auto& [this_id, dict] : id_stops) {
		for (const auto& [stop_name, leng_node] : dict->at("road_distances").AsMap()) {
			transport_catalogue_proto::StopsDistance* dist = database->add_distance();
			dist->set_from(this_id);
			dist->set_to(assigned_id[stop_name]);
			dist->set_distance(leng_node.AsDouble());
		}
	}
	database->set_last_id(id);
}

void LoadProtoBusesJSON(const std::vector<const json::Node*>& buses,
	transport_catalogue_proto::TransportProto* database,
	const std::unordered_map<std::string_view, size_t>& assigned_id) {

	for (const json::Node* bus_node : buses) {
		//void/
		transport_catalogue_proto::Buses* pb_bus = database->add_buses();
		transport_catalogue_proto::RouteInfo* pb_route = pb_bus->mutable_route();

		const json::Dict& bus_map = bus_node->AsMap();
		std::vector<std::string_view> stops;
		//std::set<size_t> uniq_stops_set_id;

		pb_bus->set_bus_name(bus_map.at("name").AsString());
		pb_route->set_ring(bus_map.at("is_roundtrip").AsBool());


		for (const json::Node& stops_node : bus_map.at("stops").AsArray()) {
			size_t id = assigned_id.at(stops_node.AsString());
			pb_route->add_the_route(id);
			//uniq_stops_set_id.insert(id);
		}
		//pb_route->set_unique_stops(uniq_stops_set_id.size());
	}

}

//Req_read

void LoadRequestJsonPB(std::istream& is,
	json::Dict* serial_settings,
	json::Array* req_array) 
{
	json::Document doc = json::Load(is);
	const json::Dict& the_map = doc.GetRoot().AsMap();

	*req_array = the_map.at("stat_requests").AsArray();

	if (the_map.count("serialization_settings")) {
		*serial_settings = the_map.at("serialization_settings").AsMap();
	}
}

}//jsonreader

//SVG MapRender	Star here
