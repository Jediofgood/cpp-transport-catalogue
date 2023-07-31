#pragma once
//класс транспортного справочника

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <set>

#include <map>

#include "geo.h"
#include "json.h"

//class Trasport_catalogue;

namespace transport_catalogue{

class TrasportCatalogue;

struct DistanceTo {
	std::string_view name_;
	double lenght_;
};

namespace print_info{

struct PrintStop{
	bool in_catalogue = false;
	std::string_view name{};
	const std::set<std::string_view, std::less<>>* ptr_set;
};

struct PrintBus {
	bool in_catalogue = false;
	std::string_view name = "";
	size_t stops = 0;
	size_t unique_stops = 0;
	double true_length = 0;
	double curvature = 0;
};

}//print_info

//Класс остановок.
class Stops {
public:
	explicit Stops(){}

	explicit Stops(std::string_view name, geo::Coordinates coordinates)
		:stop_name_(std::string{ name.begin(), name.end() }), coordinates_(coordinates)
	{}

	explicit Stops(std::string_view name, geo::Coordinates coordinates, size_t id)
		:stop_name_(std::string{ name.begin(), name.end() }), coordinates_(coordinates), id_(id)
	{}
	std::string_view GetName() const;
	size_t GetId() const;
	geo::Coordinates GetCoordinate() const;

private:
	std::string stop_name_;
	geo::Coordinates coordinates_;

	size_t id_ = 0;
};

struct RouteInfo {
	bool ring_;
	std::vector<Stops*> the_route_;
	int unique_stops_;
};

//Класс маршрутов.
class Bus {

friend transport_catalogue::TrasportCatalogue;

public:
	explicit Bus()
	{}

	explicit Bus(std::string_view bus, bool ring)
		:bus_name_(std::string{ bus.begin(), bus.end() })
	{
		route_.ring_ = ring;
	}
	
	std::string_view GetName() const;

	void AddStop(Stops* stop_ptr);

	bool IsRing() const;

	const std::vector<Stops*>& GetRoute() const;

private:
	std::string bus_name_;
	RouteInfo route_;
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

	void AddStop(std::string_view name, geo::Coordinates coordinates);

	void AddLenghtBetweenStops(std::string_view stop1, std::string_view stop2, double lenght);

	double GetLenghtBetweenStops(Stops* stop1, Stops* stop2) const;

	double GetLenghtBetweenStopsInKM(Stops* stop1, Stops* stop2) const;

	void AddBus(std::string_view bus, const std::vector<std::string_view>& stops, bool ring);

	Stops* GetStopPTR(std::string_view name);

	const print_info::PrintStop GetPrintStop(std::string_view stop) const;

	const json::Node GetJsonStopRes(std::string_view stop, int id) const;

	const print_info::PrintBus GetPrintBus(std::string_view bus) const;

	const json::Node GetJsonBusRes(std::string_view bus_name, int id) const;

	const std::deque<Stops>& GetStops() const;

	const std::map<std::string_view, Bus*> GetBuses() const;

	size_t GetLastStopId() const;

private:
	std::deque<Stops> stop_storage_;
	std::unordered_map<std::string_view, Stops*> stops_catalogue_;
	std::unordered_map<std::pair<Stops*, Stops*>, double, Hashing> true_lenght_;
	std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>> buses_on_stop_;

	std::deque<Bus> bus_storage_;
	std::map<std::string_view, Bus*> route_catalogue_;

	size_t last_id = 0;

	double CalculateLenght(Bus& bus) const;
	
	double CalculateStraightLenght(Bus& bus) const;
};

}//transport_catalogue