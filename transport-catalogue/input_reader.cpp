#include "input_reader.h"
//чтение запросов на заполнение базы;

#include <stdexcept>
#include <deque>
#include <cassert>
#include <unordered_map>

namespace input_readed {

geo::Coordinates ReadCoordinate(std::string line) {
	geo::Coordinates cord;
	size_t com_pos = line.find(',');
	cord.lat = std::stod(line.substr(0, com_pos));
	cord.lng = std::stod(line.substr(com_pos + 1));
	return cord;
}

RequestType DefineRequestType(std::string_view request) {
	using namespace std::string_view_literals;
	if (request == "Stop"sv) {
		return RequestType::Stop;
	}
	else if (request == "Bus"sv) {
		return RequestType::Bus;
	}
	else {
		throw std::invalid_argument(""); //Ќеизвестный запрос.
	}
}

//–азбиваем линию на остановки, убира€ лишнии > - и пробелы.
std::vector<std::string_view> SplitIntoWords(std::string_view str) {
	std::vector<std::string_view> result;

	str.remove_prefix(std::min(str.find_first_not_of(" "), str.size())); // удал€ем пробелы 

	const int64_t pos_end = str.npos;

	while (str.size() != 0) {
		int64_t space = str.find_first_of(">-"); //ќстановки разграничены >- => находим точный конец названи€. 

		std::string_view temp_sv = (space == pos_end ? str.substr(0, pos_end) : (str.substr(0, space))); //Ќазвание с возможными пробелами на конце.

		temp_sv.remove_suffix(temp_sv.size() - temp_sv.find_last_not_of(" ") - 1); //”бираем лишнее.

		result.push_back(temp_sv);
		str.remove_prefix(std::min(static_cast<size_t>(space), str.size()));
		str.remove_prefix(std::min(str.find_first_not_of("-> "), str.size()));
	}
	return result;
}

std::string_view StopProcessing(std::string& line, transport_catalogue::TrasportCatalogue *trc) {
	size_t pos = line.find(':');

	std::string_view name = line;
	name.remove_suffix(name.size() - pos);
	name.remove_suffix(name.size() - name.find_last_not_of(" ") - 1);

	size_t pos_after_coord = line.find(',', line.find(',') + 1);

	geo::Coordinates cord = ReadCoordinate(std::move(line.substr(pos + 1, pos_after_coord)));
	trc->AddStop(name, cord);
	line.erase(pos, pos_after_coord - pos);
	return name;
}

void BusProcessing(std::string_view bus_line, transport_catalogue::TrasportCatalogue* trc) {
	size_t pos = bus_line.find(':');
	std::string_view bus_name = bus_line.substr(0, pos); //им€ автобуса.
	bus_name.remove_suffix(bus_name.size() - bus_name.find_last_not_of(' ') -1);
	bus_line.remove_prefix(bus_name.size() + 2);
	bool ring = false;
	{
		size_t pos = bus_line.find_first_of(">-");
		if (bus_line[pos] == '>') {
			ring = true;
		}
	}

	std::vector<std::string_view> stops = SplitIntoWords(bus_line);
	trc->AddBus(bus_name, stops, ring);
}

std::vector<transport_catalogue::DistanceTo> StringSplitLenght(std::string_view strv) {
	strv.remove_prefix(std::min(strv.find_first_not_of(", "), strv.size()));
	std::vector<transport_catalogue::DistanceTo> result;
	bool flag = true;
	while (strv.size()>0 and flag) {
		size_t metr_pos = strv.find("m ");
		size_t after_to = strv.find("to ", metr_pos) + 3;
		size_t comma_or_end = strv.find(',', after_to);

		transport_catalogue::DistanceTo elem;


		elem.lenght_ = std::stod(static_cast<std::string>(strv.substr(0, metr_pos)));
		std::string_view name = strv.substr(after_to, comma_or_end - after_to);

		name.remove_prefix(name.find_first_not_of(' '));
		name.remove_suffix(name.size() - name.find_last_not_of(' ') - 1);

		elem.name_ = name;

		result.push_back(elem);

		if (comma_or_end == strv.npos) {
			flag = false;
		}
		//else
		//{
			strv.remove_prefix(comma_or_end + 1);
		//}
	}
	return result;
}

void SplitRequest(std::vector<std::string>& raw_data, transport_catalogue::TrasportCatalogue* trc) {

	std::vector<std::string_view> raw_buses;
	std::unordered_map<std::string_view, std::string_view> lenght_lines;

	raw_buses.reserve(raw_data.size());

	for (std::string& line : raw_data) {
		size_t pos = line.find(' ');
		RequestType type = DefineRequestType(line.substr(0, pos));
		line.erase(0, pos+1);

		if(type == RequestType::Stop){
			std::string_view stop_sv = StopProcessing(line, trc);
			lenght_lines.emplace(stop_sv, line);
		}
		if (type == RequestType::Bus) {
			raw_buses.push_back(line);
		}
	}
	for (auto [stop_sv, sv_line] : lenght_lines) {
		sv_line.remove_prefix(stop_sv.size());

		for (transport_catalogue::DistanceTo& elem : StringSplitLenght(sv_line)) {
			trc->AddLenghtBetweenTwoStops(stop_sv, elem.name_, elem.lenght_);
		}

		//trc->AddAllLenghtForOneStop(stop_sv, StringSplitLenght(sv_line));
	}
	for (std::string_view bus_line : raw_buses) {
		BusProcessing(bus_line, trc);
	}
}

void StartDatabase(std::istream& input, transport_catalogue::TrasportCatalogue *trc) {
	std::string line_numer;
	getline(input, line_numer);
	int requests_num = std::stoi(line_numer);

	std::vector<std::string> request_lines;
	request_lines.reserve(requests_num);

	for (size_t i = 0; i < requests_num; ++i) {
		std::string	line;
		getline(input, line);
		request_lines.push_back(line);
	}

	SplitRequest(request_lines, trc);

}

}//input_readed