#pragma once

#include <sstream>

#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>

namespace jsonreader {

void LoadJson(std::istream& is, transport_catalogue::TrasportCatalogue* trc, json::Array* req_array, json::Dict* render_map);

void LoadJson(std::istream& is,
	transport_catalogue::TrasportCatalogue* trc,
	json::Node& tr_set,
	json::Array* req_array,
	json::Dict* render_map
	);

	
void LoadFromJsonDB(const json::Array& base_array, transport_catalogue::TrasportCatalogue* trc);

void LoadStopsJson(const std::vector<const json::Node*>& stops, transport_catalogue::TrasportCatalogue* trc);

void LoadPrimaryStopInfo(const json::Node* stop_node, transport_catalogue::TrasportCatalogue* trc);

void LoadPrimaryBusInfo(const json::Node* bus_node, transport_catalogue::TrasportCatalogue* trc);

void LoadBusesJSON(const std::vector<const json::Node*>& buses, transport_catalogue::TrasportCatalogue* trc);

//protobuff start here

struct RequestVectors {
	std::vector<const json::Node*> stops;
	std::vector<const json::Node*> buses;
};

void MakeBaseJson(std::istream& is,
	json::Array* base_array,
	json::Node* tr_set,
	json::Dict* render_map,
	json::Dict* serial_settings);

void SortBaseRequest(const json::Array& base_array, RequestVectors* vecs);

void LoadProtoStopsJson(const std::vector<const json::Node*>& stops,
	transport_catalogue_proto::TransportProto* database,
	std::unordered_map<std::string_view, size_t>* ptr_assigned_id
);

void LoadProtoBusesJSON(const std::vector<const json::Node*>& buses,
	transport_catalogue_proto::TransportProto* database,
	const std::unordered_map<std::string_view, size_t>& assigned_id);

void LoadRequestJsonPB(std::istream& is,
	json::Dict* serial_settings,
	json::Array* req_array);

}//jsonreader
