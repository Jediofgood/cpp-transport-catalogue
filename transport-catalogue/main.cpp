#include "stat_reader.h"
#include "input_reader.h"

#include <iostream>

#include "Tester.h"

//≈щЄ учить и учить Google Guide Style C++;
//≈щЄ много работы

int main() {

	Test1();

	transport_catalogue::TrasportCatalogue catalogue;
	input_readed::StartDatabase(std::cin, &catalogue);
	stat_reader::StartRequesting(std::cin, std::cout, catalogue);

	int i = 0;
}
