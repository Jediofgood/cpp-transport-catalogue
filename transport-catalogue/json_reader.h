#pragma once

#include <sstream>

#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace jsonreader {

void LoadJson(std::istream& is, transport_catalogue::TrasportCatalogue* trc, json::Array* req_array, json::Dict* render_map);

void LoadJson(std::istream& is,
	transport_catalogue::TrasportCatalogue* trc,
	json::Node& tr_set,
	json::Array* req_array,
	json::Dict* render_map);

	
void LoadFromJsonDB(const json::Array& base_array, transport_catalogue::TrasportCatalogue* trc);

void LoadStopsJson(const std::vector<const json::Node*>& stops, transport_catalogue::TrasportCatalogue* trc);

void LoadPrimaryStopInfo(const json::Node* stop_node, transport_catalogue::TrasportCatalogue* trc);

void LoadPrimaryBusInfo(const json::Node* bus_node, transport_catalogue::TrasportCatalogue* trc);

void LoadBusesJSON(const std::vector<const json::Node*>& buses, transport_catalogue::TrasportCatalogue* trc);

}//jsonreader
