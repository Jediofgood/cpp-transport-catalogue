#include "request_handler.h"

#include <iostream>
#include <istream>
#include <fstream>
#include <string_view>

#include <transport_catalogue.pb.h>

#include <memory>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"

#include "domain.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void MakeBase(std::istream& is) {

	transport_catalogue_proto::CataloguePackage AllInfo;
	transport_catalogue::TrasportCatalogue trc;


	json::Array base_array;
	json::Node tr_set;
	json::Dict render_map;
	json::Dict serial_settings;

	transport_catalogue_proto::TransportProto& database = *AllInfo.mutable_catalogue();
	proto_render::RenderSettings& set = *AllInfo.mutable_rendersettings();

	jsonreader::MakeBaseJson(is, &base_array, &tr_set, &render_map, &serial_settings);

	jsonreader::LoadFromJsonDB(base_array, &trc);

	transport_router::TransportRouter tr(transport_router::StartRouter(tr_set), &trc);
	trc.PackPB(&database);

	render::FillProtoBuff(&set, render_map);

	tr.PackPB(AllInfo);

	serialization::SerializationDataBase(serial_settings, AllInfo);	
}

void ProcessRequests(std::istream& is, std::ostream& os = std::cout) {
	json::Dict serial_settings;
	json::Array req_array;

	transport_catalogue_proto::CataloguePackage AllInfo;

	jsonreader::LoadRequestJsonPB(is, &serial_settings, &req_array);

	serialization::DeserializationDataBase(serial_settings, &AllInfo);

	const transport_catalogue_proto::TransportProto& database = AllInfo.catalogue();

	transport_catalogue::TrasportCatalogue trc(database);

	transport_router::TransportRouterJSON rb(
		(transport_router::TransportRouter{ AllInfo, &trc }
	));

	rb.ProtoInitialization();

	json::Document docjson(jsonhandler::ProtoRequestProcceing(req_array, &trc, AllInfo.rendersettings(), &rb));

	json::Print(docjson, os);
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	const std::string_view mode(argv[1]);

	if (mode == "make_base"sv) {

		MakeBase(std::cin);

	}
	else if (mode == "process_requests"sv) {

		ProcessRequests(std::cin);

	}
	else {
		PrintUsage();
		return 1;
	}
}
