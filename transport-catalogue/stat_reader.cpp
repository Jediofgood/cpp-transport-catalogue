//чтение запросов на вывод и сам вывод;

#include "stat_reader.h"

#include <cassert>
#include <iomanip>


namespace stat_reader{

	std::ostream& operator<<(std::ostream& os, const transport_catalogue::print_info::PrintBus& to_print) {
		using namespace std::string_literals;
		if (to_print.in_catalogue){
			os << std::setprecision(6) <<
			"Bus "s << to_print.name << ": " <<
			to_print.stops << " stops on route, " <<
			to_print.unique_stops << " unique stops, " <<
			to_print.true_length << " route length, "
			<< to_print.curvature << " curvature"
			<< std::endl;
		}
		else {
			os << "Bus "s << to_print.name << ": not found"s << std::endl;
		}
		return os;
}

std::ostream& operator<<(std::ostream& os, const transport_catalogue::print_info::PrintStop& to_print) {
	using namespace std::string_literals;
	if (to_print.in_catalogue) {
		if (to_print.ptr_set->empty()) {
			os << "Stop " << to_print.name << ": no buses" << std::endl;
		}
		else {
			os << "Stop " << to_print.name << ": buses";
			for (std::string_view buses : *to_print.ptr_set) {
				os << " " << buses;
			}
			os << std::endl;
		}
	}
	else {
		os << "Stop " << to_print.name << ": not found" << std::endl;
	}
	return os;
}


RequestType DefineRequestType(std::string_view str) {
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

void RequestBus(std::string_view line, std::ostream& output, const transport_catalogue::TrasportCatalogue& trc) {
	using namespace std::string_literals;
	line.remove_prefix(line.find_first_not_of(' '));
	line.remove_suffix(line.size() - line.find_last_not_of(" ") - 1);

	transport_catalogue::print_info::PrintBus to_print = trc.GetPrintBus(line);

	output << to_print;
}

void RequestStop(std::string_view line, std::ostream& output, const transport_catalogue::TrasportCatalogue& trc) {
	line.remove_prefix(line.find_first_not_of(' '));
	line.remove_suffix(line.size() - line.find_last_not_of(" ") - 1);

	transport_catalogue::print_info::PrintStop to_print = trc.GetPrintStop(line);

	output << to_print;
}

//Начала обращений к БД. 
void StartRequesting(std::istream& input, std::ostream& output, const transport_catalogue::TrasportCatalogue& trc){
	std::string line;
	getline(input, line);
	int req_num = std::stoi(line);
	assert(req_num > 0);

	for (int i = 0; i < req_num; ++i) {
		std::string line;
		getline(input, line);
		size_t pos = line.find(' ');
		RequestType request = DefineRequestType(line.substr(0, pos));
		switch (request)
		{
		case stat_reader::RequestType::Bus:
			RequestBus(line.substr(pos+1), output, trc);
			break;
		case stat_reader::RequestType::Stop:
			RequestStop(line.substr(pos + 1), output, trc);
			break;
		default:
			break;
		}

	}
}


}//stat_reader