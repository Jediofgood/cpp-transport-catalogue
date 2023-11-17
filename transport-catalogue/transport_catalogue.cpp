#include "transport_catalogue.h"
#include "json_builder.h"
//класс транспортного справочника

namespace transport_catalogue {

std::string_view Stops::GetName() const{
	return stop_name_;
}

geo::Coordinates Stops::GetCoordinate() const {
	return coordinates_;
}

size_t Stops::GetId() const {
	return id_;
}

std::string_view Bus::GetName() const {
	return bus_name_;
}

void Bus::AddStop(Stops* stop_ptr) {
	route_.the_route_.push_back(stop_ptr);
}

const std::vector<Stops*>& Bus::GetRoute() const {
	return route_.the_route_;
}

bool Bus::IsRing() const {
	return route_.ring_;
}

size_t Hashing::operator()(std::pair<Stops*, Stops* >stop_pair) const {
	return hasher_(stop_pair.first) * (simple_num_ ^ 2) + hasher_(stop_pair.second) * simple_num_;
}

void TransportCatalogue::AddStop(std::string_view name, geo::Coordinates coordinates) {
	stop_storage_.push_back(std::move(Stops{ name, coordinates, last_id++ }));
	stops_catalogue_.emplace(stop_storage_.back().GetName(), &stop_storage_.back());
	buses_on_stop_[stop_storage_.back().GetName()];
}

void TransportCatalogue::AddLenghtBetweenStops(std::string_view stop1, std::string_view stop2, double lenght) {
	std::pair<Stops*, Stops*> stop_pair = std::make_pair(stops_catalogue_[stop1], stops_catalogue_[stop2]);
	true_lenght_.emplace(stop_pair, lenght);
}

double TransportCatalogue::GetLenghtBetweenStops(Stops* stop1, Stops* stop2) const {
	double leng = 0;
	std::pair<Stops*, Stops*> stop_pair1 = std::make_pair(stop1, stop2);
	if (true_lenght_.count(stop_pair1) != 0) {
		leng = true_lenght_.at(stop_pair1);
	}
	else {
		std::pair<Stops*, Stops*> stop_pair2 = std::make_pair(stop2, stop1);
		if (true_lenght_.count(stop_pair2) != 0)
		{
			leng = true_lenght_.at(stop_pair2);
		}
		else {
			leng = geo::ComputeDistance(stop1->GetCoordinate(), stop2->GetCoordinate());
		}
	}
	return leng;
}

double TransportCatalogue::GetLenghtBetweenStopsInKM(Stops* stop1,Stops* stop2) const {
	return GetLenghtBetweenStops(stop1, stop2)/1000;
}

void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector<std::string_view>& stops, bool ring) {
	bus_storage_.push_front(transport_catalogue::Bus(bus_name, ring));
	std::unordered_set<std::string_view> unique_stops_set;

	for (std::string_view stop : stops) {
		bus_storage_[0].AddStop(stops_catalogue_.at(stop));
		buses_on_stop_[stop].insert(bus_storage_[0].GetName());
		unique_stops_set.insert(stop);
	}

	bus_storage_[0].route_.unique_stops_ = unique_stops_set.size();
	route_catalogue_.insert({ bus_storage_[0].GetName(), &bus_storage_[0] });
}

Stops* TransportCatalogue::GetStopPTR(std::string_view name) {
	return stops_catalogue_.at(name);
}

double TransportCatalogue::CalculateLenght(Bus& bus) const{
	const std::vector<Stops*>& stops = bus.GetRoute();
	bool isfirst = true;
	Stops* first_s = nullptr; / against C4703 VS
	Stops* second_s;
	double leng = 0;
	for (Stops* stop : stops) {
		if (isfirst) {
			first_s = stop;
			isfirst = false;
		}
		else {
			second_s = stop;
			leng += GetLenghtBetweenStops(first_s, second_s);
			first_s = second_s;
		}
	}
	if (!bus.IsRing()) {
		for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it) {
			second_s = *it;
			leng += GetLenghtBetweenStops(first_s, second_s);
			first_s = second_s;
		}
	}
	return leng;
}

double TransportCatalogue::CalculateStraightLenght(Bus& bus) const {
	const std::vector<Stops*>& stops = bus.GetRoute();
	bool isfirst = true;
	Stops* first_s = nullptr; //against C4703 VS
	Stops* second_s;
	double leng = 0;
	for (Stops* stop : stops) {
		if (isfirst) {
			first_s = stop;
			isfirst = false;
		}
		else {
			second_s = stop;
			leng += geo::ComputeDistance(first_s->GetCoordinate(), second_s->GetCoordinate());
			first_s = second_s;
		}
	}
	if (!bus.IsRing()) {
		for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it) {
			second_s = *it;
			leng += ComputeDistance(first_s->GetCoordinate(), second_s->GetCoordinate());
			first_s = second_s;
		}
	}
	return leng;
}

const std::deque<Stops>& TransportCatalogue::GetStops() const {
	return stop_storage_;
}

const std::map<std::string_view, Bus*> TransportCatalogue::GetBuses() const {
	return route_catalogue_;
}


const print_info::PrintStop TransportCatalogue::GetPrintStop(std::string_view stop_name) const {
	print_info::PrintStop result;
	if (stops_catalogue_.count(stop_name) == 0) {
		result.in_catalogue = false;
		result.name = stop_name;
	}
	else {
		result.in_catalogue = true;
		result.name = stops_catalogue_.at(stop_name)->GetName();
		result.ptr_set = &buses_on_stop_.at(stop_name);
	}
	return result;
}

const json::Node TransportCatalogue::GetJsonStopRes(std::string_view stop_name, int id) const {
	using namespace std::string_literals;

	if (stops_catalogue_.count(stop_name) == 0) {
		return json::Builder{}.StartDict()
			.Key("error_message"s).Value("not found"s)
			.Key("request_id"s).Value(id)
			.EndDict().Build();
	}
	else {
		json::Array bus_array;
		for (std::string_view bus : buses_on_stop_.at(stop_name)) {
			bus_array.push_back(json::Node(static_cast<std::string>(bus)));
		}
		return json::Builder{}.StartDict()
			.Key("request_id"s).Value(id)
			.Key("buses"s).Value(bus_array)
			.EndDict().Build();
	}
}

const print_info::PrintBus TransportCatalogue::GetPrintBus(std::string_view bus_name) const {
	print_info::PrintBus to_print;

	if (route_catalogue_.count(bus_name) == 0) {
		to_print.in_catalogue = false;
		to_print.name = bus_name;
	}
	else {
		Bus* bus = route_catalogue_.at(bus_name);
		to_print.name = bus_name;
		to_print.in_catalogue = true;
		to_print.unique_stops = bus->route_.unique_stops_;
		to_print.true_length = CalculateLenght(*bus);
		if(bus->IsRing())		{
			to_print.stops = bus->route_.the_route_.size();
		}
		else {
			to_print.stops = bus->route_.the_route_.size() * 2 - 1;
		}
		to_print.curvature = to_print.true_length / CalculateStraightLenght(*bus);

	}
	return to_print;
}

const json::Node TransportCatalogue::GetJsonBusRes(std::string_view bus_name, int id) const {
	print_info::PrintBus to_print;

	using namespace std::string_literals;
	if (route_catalogue_.count(bus_name) == 0) {
		return json::Builder{}.StartDict()
			.Key("error_message"s).Value("not found"s)
			.Key("request_id"s).Value(id)
			.EndDict().Build();

	}
	else {
		Bus* bus = route_catalogue_.at(bus_name);
		int stops_count = 0;
		if (bus->IsRing()) {
			stops_count = bus->route_.the_route_.size();
		}
		else {
			stops_count = bus->route_.the_route_.size() * 2 - 1;
		}

		double leng = CalculateLenght(*bus);

		return json::Builder{}.StartDict()
			.Key("request_id"s).Value(id)
			.Key("curvature").Value(leng / CalculateStraightLenght(*bus))
			.Key("route_length"s).Value(leng)
			.Key("stop_count"s).Value(stops_count)
			.Key("unique_stop_count"s).Value(bus->route_.unique_stops_)
			.EndDict()
			.Build();
	}
}

size_t TransportCatalogue::GetLastStopId() const {
	return last_id;
}

std::string_view TransportCatalogue::GetBusStringView(std::string_view name) const {
	return route_catalogue_.at(name)->GetName();
}

}//transport_catalogue