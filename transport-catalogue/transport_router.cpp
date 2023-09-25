#include "transport_router.h"
#include "json_builder.h"

#include <string_view>
#include <string>
#include <deque>

namespace transport_router {

BusTimeInfo StartRouter(const json::Node& node){
	return BusTimeInfo{ node.AsMap().at("bus_wait_time").AsInt(), node.AsMap().at("bus_velocity").AsDouble() };
}

//TransportRouter::

TransportRouter::TransportRouter(BusTimeInfo info, transport_catalogue::TransportCatalogue* trc)
	:bus_wait_time_(info.bus_wait_time), bus_velocity_(info.bus_velocity), trc_(trc)
{}

double TransportRouter::CalculateTime(double leng) {
	return leng / bus_velocity_ * min_int_hour;
}

void TransportRouter::AddEdge(const transport_catalogue::Stops* stop_from, const transport_catalogue::Stops* stop_to, double leng,
	std::string_view bus_name, int span) {
	double weight = (0.0 + bus_wait_time_) + CalculateTime(leng);
	struct graph_transport_info::EdgeInfo info = {bus_name, span, leng };
	graph::Edge<double> edge = {stop_from->GetId(), stop_to->GetId(), weight, info};
	graph_.AddEdge(edge);
}

void TransportRouter::CreateGraph() {
	std::map<std::string_view, transport_catalogue::Bus*> bus_catalogue = trc_->GetBuses();

	for (auto [bus_name, bus_ptr] : bus_catalogue) {
		const std::vector<transport_catalogue::Stops*> stops = bus_ptr->GetRoute();
		for (auto it_from = stops.begin(); it_from != stops.end(); ++it_from) {

			transport_catalogue::Stops* stops_from = *it_from;

			transport_catalogue::Stops* stops_prev = *it_from;

			int span = 1;
			double leng = 0;

			for (auto it_to = it_from + 1; it_to != stops.end(); ++it_to) {
				transport_catalogue::Stops* stops_to = *it_to;
				leng += trc_->GetLenghtBetweenStopsInKM(stops_prev, stops_to);

				AddEdge(stops_from, stops_to, leng, bus_name, span);

				stops_prev = stops_to;
				++span;
			}
		}
		if (!bus_ptr->IsRing()) {
			for (auto it_from = stops.rbegin(); it_from != stops.rend(); ++it_from) {
				transport_catalogue::Stops* stops_from = *it_from;
				transport_catalogue::Stops* stops_prev = *it_from;
				int span = 1;
				double leng = 0;

				for (auto it_to = it_from + 1; it_to != stops.rend(); ++it_to) {
					transport_catalogue::Stops* stops_to = *it_to;
					leng += trc_->GetLenghtBetweenStopsInKM(stops_prev, stops_to);

					AddEdge(stops_from, stops_to, leng, bus_name, span);

					stops_prev = stops_to;
					++span;
				}
			}
		}
	}
}

void TransportRouter::Initialization() {
	graph_ = graph::DirectedWeightedGraph<double>{ trc_->GetLastStopId() };
	CreateGraph();
	opt_router_.emplace(graph::Router<double>{graph_});
}

std::optional<graph::Router<double>::RouteInfo> TransportRouter::RouteBuild(std::string_view from, std::string_view to) {
	using namespace std::string_literals;

	transport_catalogue::Stops* stop_from = trc_->GetStopPTR(from);
	transport_catalogue::Stops* stop_to = trc_->GetStopPTR(to);

	return opt_router_.value().BuildRoute(stop_from->GetId(), stop_to->GetId());
}


//TransportRouterJSON

TransportRouterJSON::TransportRouterJSON(TransportRouter tr) 
	:TransportRouter(tr)
{}

json::Node TransportRouterJSON::GetJsonRouteRes(std::string_view from, std::string_view to, int id) {
	std::optional<graph::Router<double>::RouteInfo> raw_ans = RouteBuild(from, to);
	if (!raw_ans.has_value()) {
		return CreateErrorMessage(id);
	}
	else {
		return FillAnswer(id, raw_ans.value());
	}
}

json::Node TransportRouterJSON::MakeWaitDict(const transport_catalogue::Stops* stop_from) {
	using namespace std::string_literals;
	return json::Builder{}.StartDict().Key("type"s).Value("wait"s)
		.Key("time"s).Value(bus_wait_time_)
		.Key("stop_name").Value(static_cast<std::string>(stop_from->GetName()))
		.EndDict().Build();
}

json::Node TransportRouterJSON::MakeBusDict(const graph::Edge<double>& edge) {
	using namespace std::string_literals;
	return json::Builder{}.StartDict().Key("type"s).Value("Bus"s)
		.Key("span_count"s).Value(edge.info.span)
		.Key("bus"s).Value(static_cast<std::string>(edge.info.bus_name))
		.Key("time").Value(CalculateTime(edge.info.leng)).EndDict().Build();
}

json::Node TransportRouterJSON::CreateErrorMessage(int id) {
	using namespace std::string_literals;
	return json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s).EndDict().Build();
}

json::Node TransportRouterJSON::FillAnswer(int id, const graph::Router<double>::RouteInfo raw_answer) {
	using namespace std::string_literals;

	double weight = raw_answer.weight;
	const std::deque<transport_catalogue::Stops>& stops_storage = trc_->GetStops();

	std::vector<size_t> edges = raw_answer.edges;

	std::vector<json::Node> route;

	for (size_t edge_id : edges) {
		graph::Edge<double> edge = graph_.GetEdge(edge_id);

		const transport_catalogue::Stops* stop_from = &stops_storage[edge.from];
		route.push_back(std::move(MakeWaitDict(stop_from)));
		route.push_back(std::move(MakeBusDict(edge)));

	}

	return json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("total_time"s).Value(weight).Key("items").Value(route).EndDict().Build();
}

/*
void TransportRouter::FillOptRouter(const transport_catalogue_proto::CataloguePackage& db) {
	FillGraph(db);

	//std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>> routes_internal_data;

	//const proto_grapth::Router& p_route = db.router().route();

	//std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>> routes_internal_data(p_route.routes_internal_data_size());

	//for (const proto_grapth::RoutesInternalData& vec1 : p_route.routes_internal_data()) {
		//std::vector<std::optional<graph::Router<double>::RouteInternalData>> route_internal_data(vec1.vect_size());

		//for (const proto_grapth::OptionalRouteInternalData& p_opt : vec1.vect()) {
			//if (p_opt.has_value()) {
				//const proto_grapth::RouteInternalData& p_data = p_opt.value();

				//graph::Router<double>::RouteInternalData data;
				//data.weight = p_data.weight().weight();
				//if (p_data.has_prev_edge()) {
				//	data.prev_edge = std::make_optional<size_t>(p_data.prev_edge().id());
				//}

				//route_internal_data.push_back(data);
			//}
		//}
	//}

	//graph::Router<double> rout{ graph_, routes_internal_data };

}
*/

void TransportRouter::Initialization_NoGrarh() {
	opt_router_.emplace(graph::Router<double>{graph_});
}

}//transport_router