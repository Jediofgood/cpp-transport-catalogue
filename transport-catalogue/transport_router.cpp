#include "transport_router.h"
#include "json_builder.h"

#include <string_view>
#include <string>
#include <deque>

namespace transport_router {

double TransportRouter::CalculateTime(double leng) {
	return leng / bus_velocity_ * 60;
}

TransportRouter::TransportRouter(const json::Node& set, transport_catalogue::TrasportCatalogue* trc) {
	SetNode(set);

	graph_ = graph::DirectedWeightedGraph<double>{ trc->GetLastStopId() };

	CreateGraph(trc);
}

void TransportRouter::SetNode(const json::Node& node){
	bus_velocity_ = node.AsMap().at("bus_velocity").AsDouble();
	bus_wait_time_ = node.AsMap().at("bus_wait_time").AsInt();
}

void TransportRouter::AddEdge(const transport_catalogue::Stops* stop_from, const transport_catalogue::Stops* stop_to, double leng,
	std::string_view bus_name, int span) {
	double weight = (0.0 + bus_wait_time_) + CalculateTime(leng);
	struct graph_transport_info::EdgeInfo info = {bus_name, span, leng };
	graph::Edge<double> edge = {stop_from->GetId(), stop_to->GetId(), weight, info};
	graph_.AddEdge(edge);
}

void TransportRouter::CreateGraph(transport_catalogue::TrasportCatalogue* trc) {
	std::map<std::string_view, transport_catalogue::Bus*> bus_catalogue = trc->GetBuses();

	for (auto [bus_name, bus_ptr] : bus_catalogue) {
		const std::vector<transport_catalogue::Stops*> stops = bus_ptr->GetRoute();
		for (auto it_from = stops.begin(); it_from != stops.end(); ++it_from) {

			transport_catalogue::Stops* stops_from = *it_from;

			transport_catalogue::Stops* stops_prev = *it_from;

			int span = 1;
			double leng = 0;

			for (auto it_to = it_from + 1; it_to != stops.end(); ++it_to) {
				transport_catalogue::Stops* stops_to = *it_to;
				leng += trc->GetLenghtBetweenStopsInKM(stops_prev, stops_to);

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
					leng += trc->GetLenghtBetweenStopsInKM(stops_prev, stops_to);

					AddEdge(stops_from, stops_to, leng, bus_name, span);

					stops_prev = stops_to;
					++span;
				}
			}
		}
	}
}

void TransportRouter::Initialization(transport_catalogue::TrasportCatalogue* trc) {
	CreateGraph(trc);
	opt_router_.emplace(graph::Router<double>{graph_});
}

json::Node TransportRouter::MakeWaitDict(const transport_catalogue::Stops* stop_from) {
	using namespace std::string_literals;
	return json::Builder{}.StartDict().Key("type"s).Value("wait"s)
		.Key("time"s).Value(bus_wait_time_)
		.Key("stop_name").Value(static_cast<std::string>(stop_from->GetName()))
		.EndDict().Build();
}

json::Node TransportRouter::MakeBusDict(const graph::Edge<double>& edge) {
	using namespace std::string_literals;
	return json::Builder{}.StartDict().Key("type"s).Value("Bus"s)
		.Key("span_count"s).Value(edge.info.span)
		.Key("bus"s).Value(static_cast<std::string>(edge.info.bus_name))
		.Key("time").Value(CalculateTime(edge.info.leng)).EndDict().Build();
}

json::Node TransportRouter::CreateErrorMessage(int id) {
	using namespace std::string_literals;
	return json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("error_message"s).Value("not found"s).EndDict().Build();
}

json::Node TransportRouter::CreateAnswer(int id, const graph::Router<double>::RouteInfo raw_answer, transport_catalogue::TrasportCatalogue* trc) {
	using namespace std::string_literals;

	double weight = raw_answer.weight;
	const std::deque<transport_catalogue::Stops>& stops_storage = trc->GetStops();

	std::vector<size_t> edges = raw_answer.edges;

	std::vector<json::Node> route;

	for (size_t edge_id : edges) {
		graph::Edge<double> edge = graph_.GetEdge(edge_id);

		const transport_catalogue::Stops* stop_from = &stops_storage[edge.from];
		//const transport_catalogue::Stops* stop_to = &stops_storage[edge.to];
		route.push_back(std::move(MakeWaitDict(stop_from)));
		route.push_back(std::move(MakeBusDict(edge)));

	}

	return json::Builder{}.StartDict().Key("request_id"s).Value(id).Key("total_time"s).Value(weight).Key("items").Value(route).EndDict().Build();
}


json::Node TransportRouter::RouteBuild(const json::Dict& node, transport_catalogue::TrasportCatalogue* trc) {
	using namespace std::string_literals;
	int id = node.at("id"s).AsInt();

	std::string_view from = node.at("from"s).AsString();
	std::string_view to = node.at("to"s).AsString();

	transport_catalogue::Stops* stop_from = trc->GetStopPTR(from);
	transport_catalogue::Stops* stop_to = trc->GetStopPTR(to);

	std::optional<graph::Router<double>::RouteInfo> raw_answer = opt_router_.value().BuildRoute(stop_from->GetId(), stop_to->GetId());

	if (!raw_answer.has_value()) {
		return CreateErrorMessage(id);
	}
	else {
		return CreateAnswer(id, raw_answer.value(), trc);
	}
}

}//transport_router