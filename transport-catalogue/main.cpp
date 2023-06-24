#include "request_handler.h"

#include <iostream>
#include <istream>
#include <fstream>

#include <memory>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "svg.h"

#include "domain.h"


int main() {
	transport_catalogue::TrasportCatalogue trc;
	json::Array req_array;
	json::Dict render_map;
	//std::ifstream input("inputwithpic.txt");
	jsonreader::LoadJson(std::cin, &trc, &req_array, &render_map);

	json::Document docjson(jsonhandler::RequestProcceing(req_array, &trc, render_map));
	json::Print1(docjson, std::cout);


}
