#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "json.h"

#include <memory>
#include <optional>

#include <transport_catalogue.pb.h>

namespace transport_router {

struct BusTimeInfo {
	int bus_wait_time = 0;
	double bus_velocity = 0.;
};

BusTimeInfo StartRouter(const json::Node& node);

class TransportRouter {
public:
	TransportRouter(BusTimeInfo info, transport_catalogue::TrasportCatalogue*);

	TransportRouter(const transport_catalogue_proto::CataloguePackage& db,
		transport_catalogue::TrasportCatalogue* trc);

	void Initialization();
	void ProtoInitialization();

	void PackGraphPB(proto_grapth::DirectedWeightedGraph* p_graph);

	//void PackageRoutePB(proto_grapth::Router* router);

	void PackPB(transport_catalogue_proto::CataloguePackage& db);

protected:
	static const int min_int_hour = 60;

	int bus_wait_time_ = 0;
	double bus_velocity_ = 0.;
	transport_catalogue::TrasportCatalogue* trc_;

	graph::DirectedWeightedGraph<double> graph_;
	std::optional<graph::Router<double>> opt_router_;

	double CalculateTime(double leng);

	std::optional<graph::Router<double>::RouteInfo> RouteBuild(std::string_view from, std::string_view to);

	//void FillOptRouter(const transport_catalogue_proto::CataloguePackage& db);

	void FillGraph(const transport_catalogue_proto::CataloguePackage& db);


private:
	void AddEdge(const transport_catalogue::Stops* stop_from, const transport_catalogue::Stops* stop_to, double leng,
		std::string_view bus_name, int span);

	void CreateGraph();

	void PackGraphPBEdges(proto_grapth::DirectedWeightedGraph* p_graph);
	void TransportRouter::PackGraphPBIncidenceLists(proto_grapth::DirectedWeightedGraph* p_graph);

	std::vector<graph::Edge<double>> FillEdges(const proto_grapth::DirectedWeightedGraph& graph);
	std::vector<std::vector<size_t>> IncidenceList(const proto_grapth::DirectedWeightedGraph& graph);
};

class TransportRouterJSON : public TransportRouter {
public:
	TransportRouterJSON(TransportRouter);

	json::Node GetJsonRouteRes(std::string_view from, std::string_view to, int id);

private:

	json::Node MakeWaitDict(const transport_catalogue::Stops* stop_from);

	json::Node MakeBusDict(const graph::Edge<double>& edge);

	json::Node CreateErrorMessage(int id);

	json::Node FillAnswer(int id, const graph::Router<double>::RouteInfo raw_answer);

};
}//transport_router
