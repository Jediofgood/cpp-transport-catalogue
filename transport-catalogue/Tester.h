#pragma once

#include "input_reader.h"
#include "stat_reader.h"
#include "geo.h"
#include "transport_catalogue.h"

#include <sstream>
#include <iostream>
#include <cassert>

//отдельные функции//

bool operator==(const transport_catalogue::DistanceTo& lhs, const transport_catalogue::DistanceTo& rhs) {
	return (lhs.name_ == rhs.name_) and (lhs.lenght_ == rhs.lenght_);
}

bool operator!=(const transport_catalogue::DistanceTo& lhs, const transport_catalogue::DistanceTo& rhs) {
	return (lhs.name_ != rhs.name_) or (lhs.lenght_ != rhs.lenght_);
}

void Test_String_Split_Lenght() {
	using namespace input_readed;
	{
		{
			transport_catalogue::DistanceTo t1, t2;
			t1.name_ = "Rasskazovka";
			t1.lenght_ = 19900;

			std::string line1 = "  19900m    to    Rasskazovka     ";
			std::vector<transport_catalogue::DistanceTo> vect = StringSplitLenght(line1);

			assert(vect[0] == t1);
		}
		{
			transport_catalogue::DistanceTo t1, t2;
			t1.name_ = "Rasskazovka";
			t1.lenght_ = 19900;

			t2.name_ = "Marushkino Tovarnay";
			t2.lenght_ = 100;

			std::string line1 = "19900m to Rasskazovka, 100m to Marushkino Tovarnay";
			std::vector<transport_catalogue::DistanceTo> vect = StringSplitLenght(line1);

			assert(vect[0] == t1);
			assert(vect[1] == t2);
		}
		{
			transport_catalogue::DistanceTo t1, t2;
			t1.name_ = "Rasskazovka";
			t1.lenght_ = 199.23400;

			t2.name_ = "Marushkino";
			t2.lenght_ = 100.987;

			std::string line1 = "199.23400m to Rasskazovka, 100.987m to Marushkino";
			std::vector<transport_catalogue::DistanceTo> vect = StringSplitLenght(line1);

			assert(vect[0] == t1);
			assert(vect[1] == t2);
		}
		{
			std::string line1 = "      ";
			std::vector<transport_catalogue::DistanceTo> vect = StringSplitLenght(line1);

			assert(vect.empty());
		}

	}
}

void Test_Request_type(){
	using namespace input_readed;
	assert(DefineRequestType("Stop") == RequestType::Stop);
	assert(DefineRequestType("Bus") == RequestType::Bus);
	try
	{
		DefineRequestType("Plane");
		assert(false);
	}
	catch (const std::invalid_argument&)
	{}
}



void Test1() {
	std::istringstream inputBD{
		"13\n"
		"Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
		"Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\n"
		"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
		"Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\n"
		"Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\n"
		"Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universamn\n"
		"Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
		"Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"
		"Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"
		"Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"
		"Bus 828 : Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
		"Stop Rossoshanskaya ulitsa : 55.595579, 37.605757\n"
		"Stop Prazhskaya: 55.611678, 37.603831\n"
	};

	std::istringstream input_request{
		"6\n"
		"Bus 256\n"
		"Bus 750\n"
		"Bus 751\n"
		"Stop Samara\n"
		"Stop Prazhskaya\n"
		"Stop Biryulyovo Zapadnoye\n"
	};

	std::ostringstream output{};
	std::ostringstream correct_output{
		"Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature\n"
		"Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature\n"
		"Bus 751: not found\n"
		"Stop Samara: not found\n"
		"Stop Prazhskaya: no buses\n"
		"Stop Biryulyovo Zapadnoye: buses 256 828\n"
	};

	transport_catalogue::TrasportCatalogue catalogue;
	input_readed::StartDatabase(inputBD, &catalogue);
	stat_reader::StartRequesting(input_request, output, catalogue);

	assert(output.str() == correct_output.str());
}
