#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>

#include "Tester.h"

//≈щЄ учить и учить Google Guide Style C++;
//≈щЄ много работы

void StartAllTest() {
	Test1();
	Test_String_Split_Lenght();
	Test_Request_type();

	std::cout << "Test passed" << std::endl;
}

int main() {

	StartAllTest();

	//transport_catalogue::Trasport_catalogue catalogue = input_readed::StartDatabase(std::cin);
	//stat_reader::db_request(std::cin, std::cout, catalogue);
}
