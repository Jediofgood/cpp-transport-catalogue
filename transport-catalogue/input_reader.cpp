#include "input_reader.h"
//чтение запросов на заполнение базы;

#include <stdexcept>
#include <iostream>
#include <deque>
#include <cassert>

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace input_readed{

namespace small_part_processing{
 std::vector<Length_upto> String_Split_Lenght(std::string_view strv) {
	strv.remove_prefix(std::min(strv.find_first_not_of(", "), strv.size()));
	std::vector<Length_upto> result;
	bool flag = true;
	while (!strv.empty() and flag) {
		size_t metr_pos = strv.find("m ");
		size_t after_to = strv.find("to ", metr_pos) + 3;
		size_t comma_or_end = strv.find(',', after_to);

		Length_upto elem;


		elem.lenght_ = std::stod(static_cast<std::string>(strv.substr(0, metr_pos)));
		std::string_view name = strv.substr(after_to, comma_or_end - after_to);

		name.remove_prefix(name.find_first_not_of(' '));
		name.remove_suffix(name.size() - name.find_last_not_of(' ') - 1);

		elem.name_ = name;

		result.push_back(elem);

		if (comma_or_end == strv.npos) {
			flag = false;
		}
		strv.remove_prefix(comma_or_end + 1);
	}
	return result;
}

 //Получить координты из строки +
 geo::Coordinates Coordinate(std::string line) {
	 geo::Coordinates cord;
	 size_t com_pos = line.find(',');
	 cord.lat = std::stod(line.substr(0, com_pos));
	 cord.lng = std::stod(line.substr(com_pos + 1));
	 return cord;
 }

 //Обработка типа запроса+
 RequestType Request_type(std::string_view request) {
	 if (request == "Stop"sv) {
		 return RequestType::Stop;
	 }
	 else if (request == "Bus"sv) {
		 return RequestType::Bus;
	 }
	 else {
		 throw std::invalid_argument(""); //Неизвестный запрос.
	 }
 }

 //Разбиваем линию на остановки, убирая лишнии > - и пробелы.
 std::vector<std::string_view> SplitIntoWords(std::string_view str) {
	 std::vector<std::string_view> result;

	 str.remove_prefix(std::min(str.find_first_not_of(" "), str.size())); // удаляем пробелы 

	 const int64_t pos_end = str.npos;

	 while (str.size() != 0) {
		 int64_t space = str.find_first_of(">-"); //Остановки разграничены >- => находим точный конец названия. 

		 std::string_view temp_sv = (space == pos_end ? str.substr(0, pos_end) : (str.substr(0, space))); //Название с возможными пробелами на конце.

		 temp_sv.remove_suffix(temp_sv.size() - temp_sv.find_last_not_of(" ") - 1); //Убираем лишнее.

		 result.push_back(temp_sv);
		 str.remove_prefix(std::min(static_cast<size_t>(space), str.size()));
		 str.remove_prefix(std::min(str.find_first_not_of("-> "), str.size()));
	 }
	 return result;
 }

} //small_part_processing

namespace string_line_processing {
//Сырая информация для БД
std::vector<std::string> Request_lines(std::istream& input) {

	std::string line_numer;
	getline(input, line_numer);
	int requests_num = std::stoi(line_numer);
	//assert(requests_num >= 0); //

	std::vector<std::string> request_lines;
	request_lines.reserve(requests_num);

	for (size_t i = 0; i < requests_num; ++i) {
		std::string	line;
		getline(input, line);
		request_lines.push_back(line);
	}
	return request_lines;
}

//Обработка строки - запроса с остановокой+
transport_catalogue::Stops Stop_processing(std::string& line) {
	size_t pos = line.find(':');
	std::string name = std::move(line.substr(0, pos));
	name.erase(name.find_last_not_of(" ") + 1);

	size_t pos_after_coord = line.find(',', line.find(',') + 1);

	geo::Coordinates cord = small_part_processing::Coordinate(std::move(line.substr(pos + 1, pos_after_coord)));
	transport_catalogue::Stops stop(name, cord);

	line.erase(0, pos_after_coord);
	return stop;
}

}//string_line_processing

//Запуск и заполнение БД - результат: готовая БД
transport_catalogue::Trasport_catalogue Start_database(std::istream& input){

	std::vector<std::string> raw_stops;
	std::vector<std::string> raw_buses;
	//{
	std::vector<std::string> raw_data = string_line_processing::Request_lines(input);

	raw_stops.reserve(raw_data.size());
	raw_buses.reserve(raw_data.size());
	for (std::string& line : raw_data) {
		size_t pos = line.find(' ');
		RequestType type = small_part_processing::Request_type(line.substr(0, pos));
		switch (type)
		{
		case RequestType::Stop:
			raw_stops.push_back(std::move(line.substr(pos + 1)));//StopName dobule, double (no- Stop)
			break;
		case RequestType::Bus:
			raw_buses.push_back(std::move(line.substr(pos + 1)));//BusName: stop, stop, stop ... (no - bus)
			break;
		default:
			break;
		}
	}
	raw_data.clear();
	//}
	
	//Сначала остановки.
	std::unordered_map < std::string_view, transport_catalogue::Stops*> stops_catalogue;
	std::deque<transport_catalogue::Stops> stop_storage;

	std::deque<std::string> length_stops;
	
	for (std::string& stop_line : raw_stops) {
		transport_catalogue::Stops stop = std::move(string_line_processing::Stop_processing(stop_line));

		length_stops.push_front(std::move(stop_line));//вернёмся позже. по оставшейся части не проходили.

		stop_storage.push_front(std::move(stop));
		stops_catalogue.emplace(stop_storage[0].Name(), &stop_storage[0]);	
	}
	//Все остановки в БД => заполнить расстояния.

	std::unordered_map<
		std::pair<transport_catalogue::Stops*, transport_catalogue::Stops*>, 
		size_t, 
		transport_catalogue::Hashing> 
		true_lenght_storage;

	for (size_t i = 0; i < length_stops.size(); i++) {
		using namespace transport_catalogue;

		std::vector<Length_upto> lengs_stops = std::move(
			small_part_processing::String_Split_Lenght(length_stops[i]));

		for (const Length_upto& elem : lengs_stops) {
			std::pair<Stops*, Stops*> stop_pair = std::make_pair(&stop_storage[i], stops_catalogue[elem.name_]);
			true_lenght_storage.emplace(stop_pair, elem.lenght_);
		}
		
	}

	//Остановки всё.
	
	//Теперь автобусы
	std::unordered_map<std::string_view, transport_catalogue::Bus*> bus_catalogue;
	std::deque<transport_catalogue::Bus> bus_stotage;

	for (std::string& bus_line : raw_buses) {

		size_t pos = bus_line.find(':');
		std::string bus_name = bus_line.substr(0, pos); //имя автобуса.
		bus_name.erase(bus_name.find_last_not_of(' ') + 1);
		bus_line.erase(0, bus_name.size()+2);

		transport_catalogue::Bus bus(std::move(bus_name));
		//разбиваем на остановки, и узнаём тип маршрута.

		bus_stotage.push_front(std::move(bus));

		bool ring = false;
		{
			size_t pos=bus_line.find_first_of(">-");
			if (bus_line[pos] == '>') {
				ring = true;
			}
		}

		std::vector<std::string_view> stops = small_part_processing::SplitIntoWords(bus_line);

		for (std::string_view stop : stops) {

			bus_stotage[0].AddStop(stop, stops_catalogue.at(stop));
			stops_catalogue[stop]->AddBus(bus_stotage[0].Name());
		}
		if (!ring) {
			for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it) {
				bus_stotage[0].AddStop(*it, stops_catalogue.at(*it));
			}
		}
		bus_catalogue.insert({ bus_stotage[0].Name(), &bus_stotage[0] });
	}
	//Автобусы всё
	transport_catalogue::Trasport_catalogue	return_catalogue(
		std::move(stop_storage), std::move(bus_stotage),
		std::move(stops_catalogue), std::move(bus_catalogue),
		std::move(true_lenght_storage));
	return_catalogue.Calculate_Lenght();
	return return_catalogue;
}

}//input_readed
