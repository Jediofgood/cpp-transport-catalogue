#include "transport_catalogue.h"
//класс транспортного справочника


namespace transport_catalogue {

std::string_view Stops::Name(){
	return stop_name_;
}

geo::Coordinates Stops::Coordinate() {
	return coordinates_;
}

std::string_view Bus::Name() const {
	return bus_name_;
}

size_t Bus::UniqieStops() const {
	return unique_stops_number_;
}
size_t Bus::StopsNumber() const {
	return the_route_.size();
}

double Bus::Straight_Length() const {
	return lenght_;
}

void Bus::AddStop(std::string_view stop, Stops* stop_ptr) {
	if (the_route_.size() != 0) {
		lenght_ += geo::ComputeDistance(the_route_.back()->Coordinate(), stop_ptr->Coordinate());
	}
	the_route_.push_back(stop_ptr);
	if (unique_stops_.insert(stop).second) {
		++unique_stops_number_;
	}
}

//Информация про маршрут
const std::pair<Bus*, bool> Trasport_catalogue::BusInfo(std::string_view bus) const {

	if (route_catalogue_.count(bus) == 0) {
		return { nullptr, false };
	}
	else {
		return{ route_catalogue_.at(bus), true };
	}
}

// добавить автобус на остановку.
void Stops::AddBus(std::string_view bus) {
	buses_on_stop_.insert(bus);
}
//вывести все автобусы.
const std::set<std::string_view, std::less<>>& Stops::AllBus() const {
	return buses_on_stop_;
}

const std::pair<Stops*, bool> Trasport_catalogue::StopInfo(std::string_view stop) const {
	if (stops_catalogue_.count(stop) == 0) {
		return { nullptr, false };
	}
	else {
		return{ stops_catalogue_.at(stop), true};
	}
}

bool Stops::NoBus() const {
	return buses_on_stop_.empty();
}

const std::unordered_set<std::string_view>& Bus::Stops_On_Route() const {
	return unique_stops_;
}

size_t Hashing::operator()(std::pair<Stops*, Stops* >stop_pair) const {
	return hasher_(stop_pair.first) * (simple_num_ ^ 2) + hasher_(stop_pair.second) * simple_num_;
}

const std::vector<Stops*>& Bus::The_Route() const {
	return the_route_;
}

void Trasport_catalogue::Calculate_Lenght() {
	for (Bus& bus : bus_storage_) {
		const std::vector<Stops*>& stops = bus.The_Route();
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
				else{
					std::pair<Stops*, Stops*> stop_pair2 = std::make_pair(second_s, first_s);
					if (true_lenght_.count(stop_pair2) != 0)
					{
						leng += true_lenght_.at(stop_pair2);
					}
					else {
						leng += geo::ComputeDistance(first_s->Coordinate(), second_s->Coordinate());
					}
				}
				first_s = second_s;
			}
		}
		bus.Add_True_Lenght(leng);
	}
}

double Bus::True_Route_Length() const {
	return true_lenght_;
}

void Bus::Add_True_Lenght(double leng) {
	true_lenght_ = leng;
}

}//transport_catalogue