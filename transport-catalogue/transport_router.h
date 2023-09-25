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
	TransportRouter(BusTimeInfo info, transport_catalogue::TransportCatalogue*);

	void Initialization();
	void Initialization_NoGrarh();

protected:
	static const int min_int_hour = 60;

	int bus_wait_time_ = 0;
	double bus_velocity_ = 0.;
	transport_catalogue::TransportCatalogue* trc_;

	graph::DirectedWeightedGraph<double> graph_;
	std::optional<graph::Router<double>> opt_router_;

	double CalculateTime(double leng);

	std::optional<graph::Router<double>::RouteInfo> RouteBuild(std::string_view from, std::string_view to);


//private:
protected:
	void AddEdge(const transport_catalogue::Stops* stop_from, const transport_catalogue::Stops* stop_to, double leng,
		std::string_view bus_name, int span);

	void CreateGraph();

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
