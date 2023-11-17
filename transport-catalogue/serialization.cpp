#include "serialization.h"

#include <string>
#include <string_view>
#include <map>
#include <fstream>
#include <iostream>


using namespace std::literals;

namespace serialization {

void SerializationDataBase(const json::Dict& serial_settings, 
	const transport_catalogue_proto::CataloguePackage& AllInfo) {
	std::ofstream out(serial_settings.at("file"s).AsString(), std::ios::binary);
	AllInfo.SerializeToOstream(&out);
}

void DeserializationDataBase(const json::Dict& serial_settings, transport_catalogue_proto::CataloguePackage* AllInfo) {
	std::ifstream in(serial_settings.at("file"s).AsString(), std::ios::binary);
	AllInfo->ParseFromIstream(&in);
}

}//serialization

void MakeBase(std::istream& is) {

	transport_catalogue_proto::CataloguePackage AllInfo;
	transport_catalogue::TransportCatalogueProtoBuff trc;


	json::Array base_array;
	json::Node tr_set;
	json::Dict render_map;
	json::Dict serial_settings;

	transport_catalogue_proto::TransportProto& database = *AllInfo.mutable_catalogue();

	proto_render::RenderSettings& set = *AllInfo.mutable_rendersettings();

	jsonreader::MakeBaseJson(is, &base_array, &tr_set, &render_map, &serial_settings);

	jsonreader::LoadFromJsonDB(base_array, &trc);



	transport_router::TransportRouterProtoBuff tr(transport_router::StartRouter(tr_set), &trc);
	trc.PackPB(&database);

	render::FillProtoBuff(&set, render_map);

	tr.PackPB(AllInfo);

	serialization::SerializationDataBase(serial_settings, AllInfo);
}

void ProcessRequests(std::istream& is, std::ostream& os ) {
	
	json::Dict serial_settings;
	json::Array req_array;

	transport_catalogue_proto::CataloguePackage AllInfo;

	jsonreader::LoadRequestJsonPB(is, &serial_settings, &req_array);

	serialization::DeserializationDataBase(serial_settings, &AllInfo);

	const transport_catalogue_proto::TransportProto& database = AllInfo.catalogue();

	transport_catalogue::TransportCatalogueProtoBuff trc(database);

	transport_router::TransportRouterJSON rb(
		(transport_router::TransportRouterProtoBuff{ AllInfo, &trc }
	));

	rb.Initialization_NoGrarh();

	json::Document docjson(jsonhandler::ProtoRequestProcceing(req_array, &trc, AllInfo.rendersettings(), &rb));

	json::Print(docjson, os);
	
}

namespace transport_catalogue {

TransportCatalogueProtoBuff::TransportCatalogueProtoBuff(const transport_catalogue_proto::TransportProto& db) {
	const auto& pb_stops = db.stops();
	const auto& pb_buses = db.buses();
	const auto& pb_distances = db.distance();

	std::unordered_map<size_t, std::string_view> assigned_id;

	for (const transport_catalogue_proto::Stops& stop : pb_stops) {
		AddStop(stop.stop_name(), geo::Coordinates{stop.coordinates().lat(), stop.coordinates().lon()});
		assigned_id[stop.id()] = stop.stop_name();
		//if (last_id < stop.id()) { last_id = stop.id(); }
	}

	for (const transport_catalogue_proto::StopsDistance& dist : pb_distances ) {
		AddLenghtBetweenStops(assigned_id.at(dist.from()), assigned_id.at(dist.to()), dist.distance());
	}

	for (const transport_catalogue_proto::Buses& bus: pb_buses) {
		std::vector<std::string_view> stops;
		const transport_catalogue_proto::RouteInfo route = bus.route();

		for (size_t id : route.the_route()) {
			stops.push_back(assigned_id.at(id));
		}

		AddBus(bus.bus_name(), stops, route.ring());
	}
}

void TransportCatalogueProtoBuff::PackPBStop(transport_catalogue_proto::TransportProto* db) {
	for (const Stops& stop : stop_storage_) {
		transport_catalogue_proto::Stops& s = *db->add_stops();
		s.set_id(stop.GetId());
		s.set_stop_name(std::string(stop.GetName()));
		transport_catalogue_proto::Coordinates& c = *s.mutable_coordinates();
		const geo::Coordinates& cord = stop.GetCoordinate();
		c.set_lat(cord.lat);
		c.set_lon(cord.lng);
	}
}

void TransportCatalogueProtoBuff::PackPBBus(transport_catalogue_proto::TransportProto* db) {
	for (const auto& [s_pair, leng] : true_lenght_) {
		transport_catalogue_proto::StopsDistance& d = *db->add_distance();
		d.set_from(s_pair.first->GetId());
		d.set_to(s_pair.second->GetId());
		d.set_distance(leng);
	}
}

void TransportCatalogueProtoBuff::PackPBDistance(transport_catalogue_proto::TransportProto* db) {
	for (const Bus& bus : bus_storage_) {

		transport_catalogue_proto::Buses& b = *db->add_buses();
		b.set_bus_name(bus.bus_name_);

		transport_catalogue_proto::RouteInfo& route = *b.mutable_route();

		route.set_ring(bus.IsRing());

		route.set_unique_stops(bus.route_.unique_stops_);

		for (const Stops* s_ptr : bus.route_.the_route_) {
			route.add_the_route(s_ptr->GetId());
		}
	}
}

void TransportCatalogueProtoBuff::PackPB(transport_catalogue_proto::TransportProto* db) {
	PackPBStop(db);
	PackPBBus(db);
	PackPBDistance(db);
	db->set_last_id(last_id);
}

}//transport_catalogue

namespace transport_router {

TransportRouterProtoBuff::TransportRouterProtoBuff(BusTimeInfo info, transport_catalogue::TransportCatalogue* trc)
	:TransportRouter(info, trc) {}

TransportRouterProtoBuff::TransportRouterProtoBuff(const transport_catalogue_proto::CataloguePackage& db,
	transport_catalogue::TransportCatalogue* trc)
	:TransportRouter({}, trc)
{
	UnpackGraph(db);

}

void TransportRouterProtoBuff::PackGraphPBEdges(proto_grapth::DirectedWeightedGraph* p_graph) {
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

void TransportRouterProtoBuff::PackGraphPBIncidenceLists(proto_grapth::DirectedWeightedGraph* p_graph) {
	const std::vector<std::vector<graph::EdgeId>>& incidence_lists = graph_.GetIncidenceList();
	for (const std::vector<graph::EdgeId>& incidence_list : incidence_lists) {
		proto_grapth::IncidenceList& p_list = *p_graph->add_incidence_lists();

		for (size_t edge_id : incidence_list) {
			proto_grapth::EdgeId& p_edge_id = *p_list.add_edgeid();
			p_edge_id.set_id(edge_id);
		}
	}
}

void TransportRouterProtoBuff::PackGraphPB(proto_grapth::DirectedWeightedGraph* p_graph) {
	graph_ = graph::DirectedWeightedGraph<double>{ trc_->GetLastStopId() };
	CreateGraph();
	PackGraphPBEdges(p_graph);
	PackGraphPBIncidenceLists(p_graph);
}

void TransportRouterProtoBuff::PackPB(transport_catalogue_proto::CataloguePackage& db) {

	proto_router::TransportRouter& tr_router = *db.mutable_router();

	tr_router.set_bus_wait_time(bus_wait_time_);
	tr_router.set_bus_velocity(bus_velocity_);

	proto_grapth::DirectedWeightedGraph& p_graph = *tr_router.mutable_graph();

	PackGraphPB(&p_graph);
}

std::vector<graph::Edge<double>> TransportRouterProtoBuff::FillEdges(const proto_grapth::DirectedWeightedGraph& graph) {
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

std::vector<std::vector<size_t>> TransportRouterProtoBuff::IncidenceList(const proto_grapth::DirectedWeightedGraph& graph) {
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

void TransportRouterProtoBuff::UnpackGraph(const transport_catalogue_proto::CataloguePackage& db) {

	const proto_router::TransportRouter& tr = db.router();

	bus_wait_time_ = tr.bus_wait_time();
	bus_velocity_ = tr.bus_velocity();

	const proto_grapth::DirectedWeightedGraph& graph = tr.graph();

	graph_.SetEdges(std::move(FillEdges(graph)));
	graph_.SetIncidenceList(std::move(IncidenceList(graph)));
}
}//transport_router