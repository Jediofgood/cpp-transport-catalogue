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

//Класс остановок.
class Stops {
public:
	explicit Stops(){}

	explicit Stops(std::string name, geo::Coordinates coordinates)
		:stop_name_(std::move(name)), coordinates_(coordinates)
	{}

	std::string_view Name();
	geo::Coordinates Coordinate();
	
	void AddBus(std::string_view bus);

	const std::set<std::string_view, std::less<>>& AllBus() const;

	bool NoBus() const;

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
	
	std::string_view Name() const;
	size_t UniqieStops() const;
	size_t StopsNumber() const; 

	void AddStop(std::string_view stop, Stops* stop_ptr);

	double Straight_Length() const;
	double True_Route_Length() const;

	const std::unordered_set<std::string_view>& Stops_On_Route() const;

	const std::vector<Stops*>& The_Route() const;

	void Add_True_Lenght(double leng);

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

class Trasport_catalogue {
public:
	//Запуск БД.
	explicit Trasport_catalogue(
		std::deque<transport_catalogue::Stops> stop_storage,
		std::deque<Bus> bus_storage,
		std::unordered_map<std::string_view, Stops*> stops_map,
		std::unordered_map<std::string_view, Bus*> route_catalogue,
		std::unordered_map<std::pair<Stops*, Stops*>, size_t, Hashing> true_lenght)
		:
		stop_storage_(std::move(stop_storage)), bus_storage_(std::move(bus_storage)),
		stops_catalogue_(std::move(stops_map)), route_catalogue_(std::move(route_catalogue)),
		true_lenght_(std::move(true_lenght))
	{}

	//добавление остановки в базу в сыром видел,
	void AddStop(std::string stop_line);

	//добавление остановки в базу в готовом виде.
	void AddStop(Stops stop);

	//добавление маршрута в базу,
	void AddBus(std::string stop_bus);

	//Информация про маршрут
	const std::pair<Bus*, bool> BusInfo(std::string_view bus) const;

	const std::pair<Stops*, bool> StopInfo(std::string_view stop) const;

	void Calculate_Lenght();

private:
	std::deque<transport_catalogue::Stops> stop_storage_;
	std::deque<Bus> bus_storage_;
	std::unordered_map<std::string_view, Stops*> stops_catalogue_;
	std::unordered_map<std::string_view, Bus*> route_catalogue_;
	std::unordered_map<std::pair<Stops*, Stops*>, size_t, Hashing> true_lenght_;
};


}//transport_catalogue