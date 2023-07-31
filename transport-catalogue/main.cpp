#include "request_handler.h"

#include <iostream>
#include <istream>
#include <fstream>

#include <memory>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"

#include "domain.h"


int main() {
	transport_catalogue::TrasportCatalogue trc;


	json::Array req_array;
	json::Dict render_map;
	json::Node trasport_router_set;

	std::ifstream input("newinput.json");
	jsonreader::LoadJson(std::cin, &trc, trasport_router_set, &req_array, &render_map);
	transport_router::TransportRouter rb{ trasport_router_set, &trc };

	rb.Initialization(&trc);

	json::Document docjson(jsonhandler::RequestProcceing(req_array, &trc, render_map, &rb));
	json::Print(docjson, std::cout);


}
