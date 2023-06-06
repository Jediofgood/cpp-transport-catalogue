#pragma once
//������ �������� �� ����� � ��� �����;

#include <string>
#include <iostream>
#include <sstream>

#include "transport_catalogue.h"

namespace stat_reader{

enum class RequestType {
    Bus,
    Stop,
};

std::ostream& operator<<(std::ostream& os, const transport_catalogue::print_info::PrintBus& to_print);

std::ostream& operator<<(std::ostream& os, const transport_catalogue::print_info::PrintStop& to_print);

//������ ��������� � ��. 
void StartRequesting(std::istream& input, std::ostream& output, const transport_catalogue::TrasportCatalogue& trc);

//������ - �������
void RequestBus(std::string_view line, std::ostream& output, const transport_catalogue::TrasportCatalogue& trc);

//��������� ������
RequestType DefineRequestType(std::string_view str);

//������ - ���������.
void RequestStop(std::string_view line, std::ostream& output, const transport_catalogue::TrasportCatalogue& trc);

}//stat_reader