#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "json.h"

#include <memory>
#include <optional>

namespace transport_router {

class TransportRouter {
public:

	TransportRouter(const json::Node& set, transport_catalogue::TrasportCatalogue*);

	void SetNode(const json::Node& node);

	void AddEdge(const transport_catalogue::Stops* stop_from, const transport_catalogue::Stops* stop_to, double leng,
		std::string_view bus_name, int span);

	void CreateGraph(transport_catalogue::TrasportCatalogue* trc);

	void Initialization(transport_catalogue::TrasportCatalogue* trc);

	double CalculateTime(double leng);

	json::Node MakeWaitDict(const transport_catalogue::Stops* stop_from);

	json::Node MakeBusDict(const graph::Edge<double>& edge);

	json::Node RouteBuild(const json::Dict& node, transport_catalogue::TrasportCatalogue* trc);

	json::Node CreateErrorMessage(int id);

	json::Node CreateAnswer(int id, const graph::Router<double>::RouteInfo raw_answer, transport_catalogue::TrasportCatalogue* trc);

private:				   
	int bus_wait_time_ = 0;
	double bus_velocity_ = 0.;

	graph::DirectedWeightedGraph<double> graph_;

	std::optional<graph::Router<double>> opt_router_;
};

}//transport_router
