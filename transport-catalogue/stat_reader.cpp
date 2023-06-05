//чтение запросов на вывод и сам вывод;

#include "stat_reader.h"

#include <cassert>
#include <iomanip>

namespace stat_reader{

RequestType Request_type(std::string_view str) {
	using namespace std::string_view_literals;
	if(str == "Bus"sv)
	{
		return RequestType::Bus;
	}
	if (str == "Stop"sv) {
		return RequestType::Stop;
	}
	throw std::invalid_argument("");
}

void Request_Bus(std::string_view line, std::ostream& output, const transport_catalogue::Trasport_catalogue& trc) {
	using namespace std::string_literals;
	line.remove_prefix(line.find_first_not_of(' '));
	line.remove_suffix(line.size() - line.find_last_not_of(" ") - 1);

	const std::pair < transport_catalogue::Bus*, bool>& info_bus = trc.BusInfo(line);
	
	if (info_bus.second) {
		const transport_catalogue::Bus& bus = *info_bus.first;
		output << std::setprecision(6) <<
			"Bus "s << line << ": " <<
			bus.StopsNumber() << " stops on route, " <<
			bus.UniqieStops() << " unique stops, " <<
			bus.True_Route_Length() << " route length, " 
			<< bus.True_Route_Length() / bus.Straight_Length() << " curvature"
			<< std::endl;
	}
	else {
		output << "Bus "s << line << ": not found"s << std::endl;
	}
}

void Request_Stop(std::string_view line, std::ostream& output, const transport_catalogue::Trasport_catalogue& trc) {
	line.remove_prefix(line.find_first_not_of(' '));
	line.remove_suffix(line.size() - line.find_last_not_of(" ") - 1);

	const std::pair<transport_catalogue::Stops*, bool> info_stop = trc.StopInfo(line);

	if (info_stop.second) {
		const transport_catalogue::Stops& stop = *info_stop.first;
		if (stop.NoBus()) {
			output << "Stop " << line << ": no buses" << std::endl;
		}
		else {
			output << "Stop " << line << ": buses";
			for (std::string_view buses : stop.AllBus()) {
				output << " " << buses;
			}
			output << std::endl;
		}
	}
	else {
		output << "Stop " << line << ": not found" << std::endl;
	}
}

//Начала обращений к БД. 
void db_request(std::istream& input, std::ostream& output, const transport_catalogue::Trasport_catalogue& trc){
	std::string line;
	getline(input, line);
	int req_num = std::stoi(line);
	assert(req_num > 0);

	for (int i = 0; i < req_num; ++i) {
		std::string line;
		getline(input, line);
		size_t pos = line.find(' ');
		RequestType request = Request_type(line.substr(0, pos));
		switch (request)
		{
		case stat_reader::RequestType::Bus:
			Request_Bus(line.substr(pos+1), output, trc);
			break;
		case stat_reader::RequestType::Stop:
			Request_Stop(line.substr(pos + 1), output, trc);
			break;
		default:
			break;
		}

	}
}


}//stat_reader