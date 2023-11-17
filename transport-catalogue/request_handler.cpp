#include "request_handler.h"

#include "map_renderer.h"

#include <string>
#include <sstream>
#include "json_builder.h"

namespace jsonhandler {


json::Node BusTypeRequest(const json::Node* bus_req, transport_catalogue::TransportCatalogue* trc){
	using namespace std::string_literals;
	return trc->GetJsonBusRes(bus_req->AsMap().at("name"s).AsString(), bus_req->AsMap().at("id"s).AsInt());
}

json::Node StopTypeRequest(const json::Node* stop_req, transport_catalogue::TransportCatalogue* trc) {
	using namespace std::string_literals;
	return trc->GetJsonStopRes(stop_req->AsMap().at("name"s).AsString(), stop_req->AsMap().at("id"s).AsInt());
}

json::Node RouteTypeRequest(const json::Node& route_req, transport_router::TransportRouterJSON* tr) {
	using namespace std::string_literals;

	return tr->GetJsonRouteRes(route_req.AsMap().at("from"s).AsString(), route_req.AsMap().at("to"s).AsString(), route_req.AsMap().at("id"s).AsInt());
}

json::Node SVGMapToNode(const svg::Document& svg_map, const json::Node& NodeId) {
	return render::MapToNode(svg_map, NodeId);
}

json::Node RequestProcceing(const json::Array& req_array, 
		transport_catalogue::TransportCatalogue* trc,
		const json::Dict& render_map,
		transport_router::TransportRouterJSON* tr){
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
			result.push_back(SVGMapToNode(render::MapMaker(render::FillSettings(render_map), trc), req_node));
		}
		else if (type == "Route"sv) {
			result.push_back(RouteTypeRequest(req_node, tr));
		}
		else {
			throw std::invalid_argument("");
		}
	}
	return json::Node(result);
}

//ProtoBuff Start's here

json::Node ProtoRequestProcceing(const json::Array& req_array,
	transport_catalogue::TransportCatalogue* trc,
	const proto_render::RenderSettings& rendersettings,
	transport_router::TransportRouterJSON* tr
	) {

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
			result.push_back(SVGMapToNode(render::MapMaker(render::FillSetFromProto(rendersettings), trc), req_node));
		}
		else if (type == "Route"sv) {
			result.push_back(RouteTypeRequest(req_node, tr));
		}
		else {
			throw std::invalid_argument("");
		}
	}
	return json::Node(result);
}
	
}