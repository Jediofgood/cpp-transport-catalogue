#pragma once
//чтение запросов на заполнение базы;

#include <sstream>
#include <string>
#include <vector>
//#include <unordered_map>

#include "transport_catalogue.h"
#include "geo.h"

namespace input_readed{

//Класс для опредления запроса
enum class RequestType {
    Stop,
    Bus,
};

namespace small_part_processing{

//разбиваем для true длины.
std::vector<transport_catalogue::DistanceTo> StringSplitLenght(std::string_view strv);

//Получить координты из строки
geo::Coordinates ReadCoordinate(std::string line);

//Обработка типа запроса
RequestType DefineRequestType(std::string_view request);

//Разбиваем линию на остановки, убирая лишнии > - и пробелы.
std::vector<std::string_view> SplitIntoWords(std::string_view str);

}//small_part_processing

namespace string_line_processing {

struct RawDataByType {
	explicit RawDataByType(std::vector<std::string> raw_stops, std::vector<std::string> raw_buses)
		:raw_stops_(std::move(raw_stops)), raw_buses_(std::move(raw_buses))
	{}

	std::vector<std::string> raw_stops_;
	std::vector<std::string> raw_buses_;

	//Чтоб точно.
	RawDataByType(RawDataByType&) = delete;
	RawDataByType& operator=(const RawDataByType&) = delete;

	RawDataByType(RawDataByType&&) = default;
	RawDataByType& operator=(RawDataByType&&) = default;
};

//Сырая информация для БД
std::vector<std::string> ReadRequestLines(std::istream& input);

//Обработка строки - запроса с остановокой
transport_catalogue::Stops StopProcessing(std::string& line);

//Разделяем строки по типу запроса на Stop и Bus
RawDataByType SplitRequest(std::vector<std::string> raw_data);//получаем raw_data через std::move

struct StopsCatalogue
{
	std::deque<transport_catalogue::Stops> stop_storage_;
	std::unordered_map < std::string_view, transport_catalogue::Stops*> stops_catalogue_;
	std::unordered_map<
		std::pair<transport_catalogue::Stops*, transport_catalogue::Stops*>,
		size_t, transport_catalogue::Hashing> true_lenght_;
	std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>> buses_on_stop_;

	StopsCatalogue(
		std::deque<transport_catalogue::Stops> stop_storage,
		std::unordered_map < std::string_view, transport_catalogue::Stops*> stops_catalogue,
		std::unordered_map<
		std::pair<transport_catalogue::Stops*, transport_catalogue::Stops*>,
		size_t, transport_catalogue::Hashing> true_lenght,
		std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>> buses_on_stop
	)
		:stop_storage_(std::move(stop_storage)), 
		 stops_catalogue_(std::move(stops_catalogue)), 
		 true_lenght_(std::move(true_lenght)),
		 buses_on_stop_(buses_on_stop)
	{}

	StopsCatalogue(StopsCatalogue&) = delete;
	StopsCatalogue& operator=(const StopsCatalogue&) = delete;

	StopsCatalogue(StopsCatalogue&&) = default;
	StopsCatalogue& operator=(StopsCatalogue&&) = default;
};

//Обрабатываем все остановки - возвращаем в готовом виде.
void MakeStopsCatalogue(std::vector<std::string> raw_stops, transport_catalogue::TrasportCatalogue* trc);//получаем raw_stops через std::move

void PushBusToCatalogue(std::vector<std::string> raw_buses, transport_catalogue::TrasportCatalogue* trc);//получаем raw_stops через std::move

struct BusCatalogue
{
	std::deque<transport_catalogue::Bus> bus_storage_;
		std::unordered_map<std::string_view, transport_catalogue::Bus*> bus_catalogue_;

	BusCatalogue(
		std::deque<transport_catalogue::Bus> bus_storage,
	std::unordered_map<std::string_view, transport_catalogue::Bus*> bus_catalogue) 
	:bus_storage_(bus_storage), bus_catalogue_(bus_catalogue)
	{}

	BusCatalogue(BusCatalogue&) = delete;
	BusCatalogue& operator=(const BusCatalogue&) = delete;

	BusCatalogue(BusCatalogue&&) = default;
	BusCatalogue& operator=(BusCatalogue&&) = default;
};

} //string_line_processing



//Запуск и заполнение БД - результат: готовая БД
transport_catalogue::TrasportCatalogue StartDatabase(std::istream& input);

}//input_readed

