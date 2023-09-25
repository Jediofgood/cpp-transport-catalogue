#pragma once

#include <transport_catalogue.pb.h>

#include "json.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"

#include "domain.h"
#include "serialization.h"

#include "request_handler.h"

#include <iostream>

namespace serialization{

void SerializationDataBase(const json::Dict& serial_settings, const transport_catalogue_proto::CataloguePackage& AllInfo);

void DeserializationDataBase(const json::Dict& serial_settings, transport_catalogue_proto::CataloguePackage* AllInfo);

}//serialization

void MakeBase(std::istream& is);

void ProcessRequests(std::istream& is, std::ostream& os = std::cout);

namespace transport_catalogue {

class TransportCatalogueProtoBuff : public TransportCatalogue {
public:
	explicit TransportCatalogueProtoBuff() {}
	explicit TransportCatalogueProtoBuff(const transport_catalogue_proto::TransportProto& db);//proto
	void PackPB(transport_catalogue_proto::TransportProto* db);
private:
	void PackPBStop(transport_catalogue_proto::TransportProto* db);
	void PackPBBus(transport_catalogue_proto::TransportProto* db);
	void PackPBDistance(transport_catalogue_proto::TransportProto* db);
};

}//transport_catalogue

namespace transport_router {

class TransportRouterProtoBuff : public TransportRouter {
public:

	TransportRouterProtoBuff(BusTimeInfo info, transport_catalogue::TransportCatalogue* trc);

	TransportRouterProtoBuff(const transport_catalogue_proto::CataloguePackage& db,
		transport_catalogue::TransportCatalogue* trc);

	void UnpackGraph(const transport_catalogue_proto::CataloguePackage& db);

	void PackGraphPB(proto_grapth::DirectedWeightedGraph* p_graph);
	
	//void PackRouterPB(proto_router::TransportRouter* tr_router);

	void PackPB(transport_catalogue_proto::CataloguePackage& db);

	std::vector<graph::Edge<double>> FillEdges(const proto_grapth::DirectedWeightedGraph& graph);
	std::vector<std::vector<size_t>> IncidenceList(const proto_grapth::DirectedWeightedGraph& graph);

private:
	void PackGraphPBEdges(proto_grapth::DirectedWeightedGraph* p_graph);
	void PackGraphPBIncidenceLists(proto_grapth::DirectedWeightedGraph* p_graph);

	//void UnpackRouterPB(const transport_catalogue_proto::CataloguePackage& db);
};


}//transport_router