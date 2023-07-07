#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "svg.h"

namespace jsonhandler {

json::Node RequestProcceing(const json::Array& req_array, transport_catalogue::TrasportCatalogue* trc, const json::Dict& render_map);

json::Node StopTypeRequest(const json::Node* stop_req, transport_catalogue::TrasportCatalogue* trc);

json::Node BusTypeRequest(const json::Node* bus_req, transport_catalogue::TrasportCatalogue* trc);

json::Node SVGMapToNode(const svg::Document& svg_map, const json::Node& NodeId);

}
