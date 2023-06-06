#include "transport_catalogue.h"
//класс транспортного справочника


namespace transport_catalogue {

std::string_view Stops::GetName(){
	return stop_name_;
}

geo::Coordinates Stops::GetCoordinate() {
	return coordinates_;
}

// добавить автобус на остановку.
void Stops::AddBus(std::string_view bus) {
	buses_on_stop_.insert(bus);
}

//вывести все автобусы.
const std::set<std::string_view, std::less<>>& Stops::GetAllBusSet() const {
	return buses_on_stop_;
}

bool Stops::IsNoBus() const {
	return buses_on_stop_.empty();
}

std::string_view Bus::GetName() const {
	return bus_name_;
}

size_t Bus::GetUniqieStops() const {
	return unique_stops_number_;
}
size_t Bus::GetStopsNumber() const {
	return the_route_.size();
}

void Bus::AddStop(std::string_view stop, Stops* stop_ptr) {
	if (the_route_.size() != 0) {
		lenght_ += geo::ComputeDistance(the_route_.back()->GetCoordinate(), stop_ptr->GetCoordinate());
	}
	the_route_.push_back(stop_ptr);
	if (unique_stops_.insert(stop).second) {
		++unique_stops_number_;
	}
}

double Bus::GetStraightLength() const {
	return lenght_;
}

double Bus::GetTrueRouteLength() const {
	return true_lenght_;
}

const std::unordered_set<std::string_view>& Bus::GetStopsOnRoute() const {
	return unique_stops_;
}

const std::vector<Stops*>& Bus::GetRoute() const {
	return the_route_;
}

void Bus::AddTrueLenght(double leng) {
	true_lenght_ = leng;
}

size_t Hashing::operator()(std::pair<Stops*, Stops* >stop_pair) const {
	return hasher_(stop_pair.first) * (simple_num_ ^ 2) + hasher_(stop_pair.second) * simple_num_;
}

void TrasportCatalogue::AddBus(std::string bus_name, const std::vector<std::string_view>& stops, bool ring) {

	transport_catalogue::Bus bus(std::move(bus_name));
	bus_storage_.push_front(std::move(bus));

	for (std::string_view stop : stops) {

		bus_storage_[0].AddStop(stop, stops_catalogue_.at(stop));
		//stops_catalogue_[stop]->AddBus(bus_storage_[0].GetName());
		buses_on_stop_[stop].insert(bus_storage_[0].GetName());
	}
	if (!ring) {
		for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it) {
			bus_storage_[0].AddStop(*it, stops_catalogue_.at(*it));
		}
	}
	route_catalogue_.insert({ bus_storage_[0].GetName(), &bus_storage_[0] });
	CalculateLenght(bus_storage_[0]);
}

void TrasportCatalogue::CalculateLenght(Bus& bus) {
	const std::vector<Stops*>& stops = bus.GetRoute();
	bool isfirst = true;
	Stops* first_s;
	Stops* second_s;
	double leng = 0;
	for (Stops* stop : stops) {
		if (isfirst) {
			first_s = stop;
			isfirst = false;
		}
		else {
			second_s = stop;
			std::pair<Stops*, Stops*> stop_pair1 = std::make_pair(first_s, second_s);
			if (true_lenght_.count(stop_pair1) != 0) {
				leng += true_lenght_.at(stop_pair1);
			}
			else {
				std::pair<Stops*, Stops*> stop_pair2 = std::make_pair(second_s, first_s);
				if (true_lenght_.count(stop_pair2) != 0)
				{
					leng += true_lenght_.at(stop_pair2);
				}
				else {
					leng += geo::ComputeDistance(first_s->GetCoordinate(), second_s->GetCoordinate());
				}
			}
			first_s = second_s;
		}
	}
	bus.AddTrueLenght(leng);
}

void TrasportCatalogue::CalculateLenghtForAll() {
	for (Bus& bus : bus_storage_) {
		CalculateLenght(bus);
	}
}

const print_info::PrintStop TrasportCatalogue::GetPrintStop(std::string_view stop_name) const {
	print_info::PrintStop result;
	if (stops_catalogue_.count(stop_name) == 0) {
		result.in_catalogue = false;
		result.name = stop_name;
	}
	else {
		result.in_catalogue = true;
		result.name = stops_catalogue_.at(stop_name)->GetName();
		result.ptr_set = &buses_on_stop_.at(result.name);
	}
	return result;

}

const print_info::PrintBus TrasportCatalogue::GetPrintBus(std::string_view bus_name) const {
	print_info::PrintBus to_print;

	if (route_catalogue_.count(bus_name) == 0) {
		to_print.in_catalogue = false;
		to_print.name = bus_name;
		//return to_print;
	}
	else {
		Bus* bus = route_catalogue_.at(bus_name);
		to_print.in_catalogue = true;

		to_print.name = bus->GetName();
		to_print.length = bus->GetTrueRouteLength();
		to_print.curvature = to_print.length / bus->GetStraightLength();
		to_print.stops = bus->GetStopsNumber();
		to_print.unique_stops = bus->GetUniqieStops();
	}
	return to_print;
}

}//transport_catalogue