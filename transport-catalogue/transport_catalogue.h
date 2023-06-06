#pragma once
//класс транспортного справочника

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <set>

#include "geo.h"

//class Trasport_catalogue;

namespace transport_catalogue{

namespace print_info{

struct PrintStop{
	bool in_catalogue = false;
	std::string_view name = "";
	const std::set<std::string_view, std::less<>>* ptr_set;
};

struct PrintBus {
	bool in_catalogue = false;
	std::string_view name = "";
	size_t stops = 0;
	size_t unique_stops = 0;
	double length = 0;
	double curvature = 0;
};


}//print_info

//Класс остановок.
class Stops {
public:
	explicit Stops(){}

	explicit Stops(std::string name, geo::Coordinates coordinates)
		:stop_name_(std::move(name)), coordinates_(coordinates)
	{}

	std::string_view GetName();
	geo::Coordinates GetCoordinate();
	
	void AddBus(std::string_view bus);

	const std::set<std::string_view, std::less<>>& GetAllBusSet() const;

	bool IsNoBus() const;

private:
	std::string stop_name_;
	geo::Coordinates coordinates_;

	std::set<std::string_view, std::less<>> buses_on_stop_;
};

//Класс маршрутов.
class Bus {
	//friend Trasport_catalogue;
public:

	explicit Bus()
	{}

	explicit Bus(std::string bus)
		:bus_name_(bus) 
	{}
	
	std::string_view GetName() const;
	size_t GetUniqieStops() const;
	size_t GetStopsNumber() const; 

	void AddStop(std::string_view stop, Stops* stop_ptr);

	double GetStraightLength() const;
	double GetTrueRouteLength() const;

	const std::unordered_set<std::string_view>& GetStopsOnRoute() const;

	const std::vector<Stops*>& GetRoute() const;

	void AddTrueLenght(double leng);

private:
	std::string bus_name_;
	size_t unique_stops_number_ = 0;
	long double lenght_ = 0;
	long double true_lenght_ = 0;

	std::vector<Stops*> the_route_;
	std::unordered_set<std::string_view> unique_stops_;
};

struct Hashing {
public:
	size_t operator()(std::pair<Stops*, Stops* >stop_pair) const;
private:
	std::hash<const void*> hasher_;
	const size_t simple_num_ = 23;
};

class TrasportCatalogue {
public:
	//Загружаем все остановки, т.к. остановки зависят от остановок, и автобусы зависят от остановок.
	//Воизбежания конфликтов, проблем проще обработать сразу все остановки и добавить.
	explicit TrasportCatalogue(
		std::deque<transport_catalogue::Stops> stop_storage,
		std::unordered_map<std::string_view, Stops*> stops_map,
		std::unordered_map<std::pair<Stops*, Stops*>, size_t, Hashing> true_lenght,
		std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>> buses_on_stop
		)
		: stop_storage_(std::move(stop_storage)), stops_catalogue_(std::move(stops_map)), 
		  true_lenght_(std::move(true_lenght)), buses_on_stop_(buses_on_stop)
	{
	}

	void AddBus(std::string bus, const std::vector<std::string_view>& stops, bool ring);

	//Информация про маршрут
	const std::pair<Bus*, bool> GetBusInfo(std::string_view bus) const;

	const std::pair<Stops*, bool> GetStopInfo(std::string_view stop) const;

	void CalculateLenght(Bus& bus);

	void CalculateLenghtForAll();

	const print_info::PrintStop GetPrintStop(std::string_view stop) const;

	const print_info::PrintBus GetPrintBus(std::string_view bus) const;

private:
	std::deque<Stops> stop_storage_;
	std::unordered_map<std::string_view, Stops*> stops_catalogue_;
	std::unordered_map<std::pair<Stops*, Stops*>, size_t, Hashing> true_lenght_;
	std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>> buses_on_stop_;

	std::deque<Bus> bus_storage_;
	std::unordered_map<std::string_view, Bus*> route_catalogue_;
};


}//transport_catalogue