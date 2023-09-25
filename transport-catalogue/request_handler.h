#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"

#include "json.h"
#include "svg.h"

namespace jsonhandler {

json::Node RequestProcceing(
	const json::Array& req_array, 
	transport_catalogue::TrasportCatalogue* 
	trc, const json::Dict& render_map,
	transport_router::TransportRouterJSON* tr);

json::Node StopTypeRequest(const json::Node* stop_req, transport_catalogue::TrasportCatalogue* trc);

json::Node BusTypeRequest(const json::Node* bus_req, transport_catalogue::TrasportCatalogue* trc);

json::Node RouteTypeRequest(const json::Node* bus_req,  transport_router::TransportRouter* tr);

json::Node SVGMapToNode(const svg::Document& svg_map, const json::Node& NodeId);

//protobuff here

json::Node ProtoRequestProcceing(const json::Array& req_array,
	transport_catalogue::TrasportCatalogue* trc,
	const proto_render::RenderSettings& rendersettings,
	transport_router::TransportRouterJSON* tr);

}
