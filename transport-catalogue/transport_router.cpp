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

TransportRouter::TransportRouter(BusTimeInfo info, transport_catalogue::TrasportCatalogue* trc)
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

//ProtoBuff Start here

void TransportRouter::PackGraphPBEdges(proto_grapth::DirectedWeightedGraph* p_graph) {
	const std::vector<graph::Edge<double>>& edges = graph_.GetEdges();
	for (const graph::Edge<double>& edge : edges) {
		proto_grapth::Edge& p_edge = *p_graph->add_edge();
		p_edge.set_from(edge.from);
		p_edge.set_to(edge.to);
		(*p_edge.mutable_weight()).set_weight(edge.weight);

		proto_grapth::EdgeInfo& p_info = *p_edge.mutable_info();
		const graph_transport_info::EdgeInfo& info = edge.info;

		p_info.set_bus_name(std::string{ info.bus_name });
		p_info.set_span(info.span);
		p_info.set_leng(info.leng);
	}
}

void TransportRouter::PackGraphPBIncidenceLists(proto_grapth::DirectedWeightedGraph* p_graph) {
	const std::vector<std::vector<graph::EdgeId>>& incidence_lists = graph_.GetIncidenceList();
	for (const std::vector<graph::EdgeId>& incidence_list : incidence_lists) {
		proto_grapth::IncidenceList& p_list = *p_graph->add_incidence_lists();

		for (size_t edge_id : incidence_list) {
			proto_grapth::EdgeId& p_edge_id = *p_list.add_edgeid();
			p_edge_id.set_id(edge_id);
		}
	}
}

void TransportRouter::PackGraphPB(proto_grapth::DirectedWeightedGraph* p_graph) {
	graph_ = graph::DirectedWeightedGraph<double>{ trc_->GetLastStopId() };
	CreateGraph();
	PackGraphPBEdges(p_graph);
	PackGraphPBIncidenceLists(p_graph);
}

void TransportRouter::PackPB(transport_catalogue_proto::CataloguePackage& db) {

	proto_router::TransportRouter& TRrouter = *db.mutable_router();

	TRrouter.set_bus_wait_time(bus_wait_time_);
	TRrouter.set_bus_velocity(bus_velocity_);

	proto_grapth::DirectedWeightedGraph& p_graph = *TRrouter.mutable_graph();

	PackGraphPB(&p_graph);
}

std::vector<graph::Edge<double>> TransportRouter::FillEdges(const proto_grapth::DirectedWeightedGraph& graph) {
	std::vector<graph::Edge<double>> edges;
	for (const proto_grapth::Edge& p_edge : graph.edge()) {

		const proto_grapth::EdgeInfo& p_ed_id = p_edge.info();

		graph_transport_info::EdgeInfo info{
			trc_->GetBusStringView(p_ed_id.bus_name()),
			p_ed_id.span(),
			p_ed_id.leng()
		};

		graph::Edge<double> edge{ p_edge.from(), p_edge.to(), p_edge.weight().weight(), info };
		edges.push_back(edge);
	}
	return edges;
}

std::vector<std::vector<size_t>> TransportRouter::IncidenceList(const proto_grapth::DirectedWeightedGraph& graph) {
	std::vector<std::vector<size_t>> incidence_lists;
	incidence_lists.reserve(graph.incidence_lists_size());

	for (const proto_grapth::IncidenceList p_list : graph.incidence_lists()) {

		std::vector<size_t> under_list;
		under_list.reserve(p_list.edgeid_size());

		for (const proto_grapth::EdgeId p_id : p_list.edgeid()) {
			under_list.push_back(p_id.id());
		}

		incidence_lists.push_back(under_list);
	}
	return incidence_lists;
}

void TransportRouter::FillGraph(const transport_catalogue_proto::CataloguePackage& db) {
	//graph::DirectedWeightedGraph<double> result;

	const proto_router::TransportRouter& tr = db.router();

	bus_wait_time_ = tr.bus_wait_time();
	bus_velocity_ = tr.bus_velocity();

	const proto_grapth::DirectedWeightedGraph& graph = tr.graph();

	graph_.SetEdges(std::move(FillEdges(graph)));
	graph_.SetIncidenceList(std::move(IncidenceList(graph)));	
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

void TransportRouter::ProtoInitialization() {
	opt_router_.emplace(graph::Router<double>{graph_});
}

TransportRouter::TransportRouter(const transport_catalogue_proto::CataloguePackage& db,
	transport_catalogue::TrasportCatalogue* trc) 
{
	trc_ = trc;
	FillGraph(db);
}



}//transport_router