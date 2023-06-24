#include "request_handler.h"

#include "map_renderer.h"

#include <execution>
#include <string>
#include <sstream>

namespace jsonhandler {

json::Node BusTypeRequest(const json::Node* bus_req, transport_catalogue::TrasportCatalogue* trc){

	using namespace std::string_literals;

	json::Dict result;
	result["request_id"s] = bus_req->AsMap().at("id"s).AsInt();
	std::string s = bus_req->AsMap().at("name"s).AsString();
	transport_catalogue::print_info::PrintBus bus_info = trc->GetPrintBus(bus_req->AsMap().at("name"s).AsString());

	if (bus_info.in_catalogue) {
		result["curvature"s] = json::Node(bus_info.curvature);
		result["route_length"s] = json::Node(bus_info.true_length);
		result["stop_count"s] = json::Node(static_cast<int> (bus_info.stops));
		result["unique_stop_count"s] = json::Node(static_cast<int>(bus_info.unique_stops));
	}
	else {
		result["error_message"s] = json::Node("not found"s);
	}
	return json::Node(result);
}

json::Node StopTypeRequest(const json::Node* stop_req, transport_catalogue::TrasportCatalogue* trc) {

	using namespace std::string_literals;

	json::Dict result;
	result["request_id"s] = stop_req->AsMap().at("id"s).AsInt();
	transport_catalogue::print_info::PrintStop stop_info = trc->GetPrintStop(stop_req->AsMap().at("name"s).AsString());
	if (stop_info.in_catalogue) {
		//if (!(*stop_info.ptr_set).empty()) //Ничего не сказанно?
		//{
			json::Array bus_array;
			for (std::string_view bus : *stop_info.ptr_set) {
				bus_array.push_back(json::Node(static_cast<std::string>(bus)));
			}
			result["buses"s] = json::Node(std::move(bus_array));
		//}
	}
	else {
		result["error_message"s] = json::Node("not found"s);
	}
	return json::Node(result);
}

json::Node SVGMapToNode(const svg::Document& svg_map, const json::Node& NodeId) {
	using namespace std::string_literals;
	json::Dict result;
	result["request_id"s] = NodeId.AsMap().at("id"s).AsInt();
	std::ostringstream output{};
	svg_map.Render(output);
	result["map"] = output.str();
	return json::Node{ result };
}

json::Node RequestProcceing(const json::Array& req_array, transport_catalogue::TrasportCatalogue* trc, const json::Dict& render_map){
	using namespace std::string_view_literals;

	json::Array result;

	for (const json::Node& req_node : req_array) {
		const json::Dict req_map = req_node.AsMap();
		std::string_view type = req_map.at("type").AsString();
		if (type == "Bus"sv) {
			result.push_back(std::move(BusTypeRequest(&req_node, trc)));
		}
		else if (type == "Stop"sv) {
			result.push_back(std::move(StopTypeRequest(&req_node, trc)));
		}
		else if (type == "Map"sv) {
			result.push_back(SVGMapToNode(render::MapMaker(render_map, trc), req_node));
		}
		else {
			throw std::invalid_argument("");
		}
	}
	return json::Node(result);
}

}