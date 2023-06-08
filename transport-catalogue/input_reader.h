#pragma once

#include <sstream>
#include <string>
#include <vector>
//#include <unordered_map>

#include "transport_catalogue.h"
#include "geo.h"

namespace input_readed {

enum class RequestType {
    Stop,
    Bus,
};

geo::Coordinates ReadCoordinate(std::string line);

RequestType DefineRequestType(std::string_view request);

std::vector<std::string_view> SplitIntoWords(std::string_view str);

transport_catalogue::Stops* StopProcessing(std::string& line, transport_catalogue::TrasportCatalogue* trc);

void BusProcessing(std::string_view bus_line, transport_catalogue::TrasportCatalogue* trc);

std::vector<transport_catalogue::DistanceTo> StringSplitLenght(std::string_view strv);

void AddDistance(std::unordered_map<transport_catalogue::Stops*, std::string&>& lenght_lines, transport_catalogue::TrasportCatalogue* trc);

void SplitRequest(std::vector<std::string>& raw_data, transport_catalogue::TrasportCatalogue* trc);

void StartDatabase(std::istream& input, transport_catalogue::TrasportCatalogue * trc);

}//input_readed
