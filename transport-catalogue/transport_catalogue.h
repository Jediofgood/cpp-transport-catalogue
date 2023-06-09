#pragma once
//����� ������������� �����������

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

//����� ���������.
class Stops {
public:
	explicit Stops(){}

	explicit Stops(std::string_view name, geo::Coordinates coordinates)
		:stop_name_(std::string{ name.begin(), name.end() }), coordinates_(coordinates)
	{
		//info_.name = stop_name_;
	}
	//�� ������������ ��� ������. ����� ��� ���������� �������.
	std::string_view GetName();
	//�� ������������ ��� ������. ����� ��� ���������� �������.
	geo::Coordinates GetCoordinate();

private:
	std::string stop_name_;
	geo::Coordinates coordinates_;
};

struct RouteInfo {
	bool ring_;
	std::vector<Stops*> the_route_;
	size_t unique_stops_;
};

//����� ���������.
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

	void AddStop(std::string_view stop, Stops* stop_ptr);

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

	//void AddAllLenghtForOneStop(Stops* stop, std::vector<DistanceTo>);

	//void AddAllLenghtForOneStop(std::string_view stop, std::vector<DistanceTo>);

	//void AddLenghtBetweenTwoStops(Stops* stop1, Stops* stop2, double lenght);

	void AddLenghtBetweenTwoStops(std::string_view stop1, std::string_view stop2, double lenght);

	double GetLenghtBetweenStops(Stops* stop1, Stops* stop2) const;

	void AddBus(std::string_view bus, const std::vector<std::string_view>& stops, bool ring);

	const print_info::PrintStop GetPrintStop(std::string_view stop) const;

	const print_info::PrintBus GetPrintBus(std::string_view bus) const;

private:
	std::deque<Stops> stop_storage_;
	std::unordered_map<std::string_view, Stops*> stops_catalogue_;
	std::unordered_map<std::pair<Stops*, Stops*>, double, Hashing> true_lenght_;
	std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>> buses_on_stop_;

	std::deque<Bus> bus_storage_;
	std::unordered_map<std::string_view, Bus*> route_catalogue_;

	double CalculateLenght(Bus& bus) const;
	
	double CalculateStraightLenght(Bus& bus) const;
};
}//transport_catalogue