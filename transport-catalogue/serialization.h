#pragma once

#include <transport_catalogue.pb.h>

#include "json.h"

namespace serialization{

void SerializationDataBase(const json::Dict& serial_settings, const transport_catalogue_proto::CataloguePackage& AllInfo);

void DeserializationDataBase(const json::Dict& serial_settings, transport_catalogue_proto::CataloguePackage* AllInfo);

}//serialization