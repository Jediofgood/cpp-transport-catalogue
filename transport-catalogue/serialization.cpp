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
	//if (out.fail()) {
	//cerr << "Can't open to Serialization" << std::endl;
	//throw{}; 
	//}
	AllInfo.SerializeToOstream(&out);
}

void DeserializationDataBase(const json::Dict& serial_settings, transport_catalogue_proto::CataloguePackage* AllInfo) {
	std::ifstream in(serial_settings.at("file"s).AsString(), std::ios::binary);
	//if (out.fail()) {
	//cerr << "Can't open to Serialization" << std::endl;
	//throw{}; 
	//}
	AllInfo->ParseFromIstream(&in);
}

}//serialization