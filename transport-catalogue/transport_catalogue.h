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

class DistanceTo {
public:
	std::string_view name_;
	double lenght_;
};

namespace print_info{

struct InfoStop{
	std::string_view name{};
};

struct InfoBus
{
	std::string_view name;
	size_t stops = 0;
	size_t unique_stops = 0;
	double true_length = 0;
	double straight_length = 0;
	double curvature = 0;
};

struct PrintStop{
	bool in_catalogue = false;
	InfoStop info;
	const std::set<std::string_view, std::less<>>* ptr_set;
};

struct PrintBus {
	bool in_catalogue = false;
	std::string_view name = "";
	InfoBus info;
};

}//print_info

//Класс остановок.
class Stops {
public:
	explicit Stops(){}

	explicit Stops(std::string_view name, geo::Coordinates coordinates)
		:stop_name_(std::string{ name.begin(), name.end() }), coordinates_(coordinates)
	{
		//info_.name = stop_name_;
	}
	//не используется для печати. нужен для внутренних функций.
	std::string_view GetName();
	//не используется для печати. нужен для внутренних функций.
	geo::Coordinates GetCoordinate();

	print_info::InfoStop GetInfo();

private:
	std::string stop_name_;
	geo::Coordinates coordinates_;
	print_info::InfoStop info_;
};

struct RouteInfo {
	bool ring_;
	std::vector<Stops*> the_route_;
	std::unordered_set<std::string_view> unique_stops_set_;
};

//Класс маршрутов.
class Bus {
	//friend Trasport_catalogue;
public:
	explicit Bus()
	{}

	explicit Bus(std::string_view bus, bool ring)
		:bus_name_(std::string{ bus.begin(), bus.end() })
	{
		route_.ring_ = ring;
	}
	
	//не используется для печати. нужен для внутренних функций.
	std::string_view GetName() const;

	void AddStop(std::string_view stop, Stops* stop_ptr);

	const std::unordered_set<std::string_view>& GetStopsOnRoute() const;

	const std::vector<Stops*>& GetRoute() const;

	void AddTrueLenght(double leng);

	void FillInfo();

	print_info::InfoBus GetBus() const;

private:
	std::string bus_name_;

	RouteInfo route_;
	
	//bool ring_;
	//std::vector<Stops*> the_route_;
	//std::unordered_set<std::string_view> unique_stops_;

	print_info::InfoBus info_;
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
	explicit TrasportCatalogue()
	{}

	Stops* AddStop(Stops stop);

	void AddLenght(Stops* stop, std::vector<DistanceTo>);

	void AddBus(std::string_view bus, const std::vector<std::string_view>& stops, bool ring);

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