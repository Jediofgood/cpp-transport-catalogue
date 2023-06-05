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

//������ ��������� � ��. 
void db_request(std::istream& input, std::ostream& output, const transport_catalogue::Trasport_catalogue& trc);

//������ - �������
void Request_Bus(std::string_view line, std::ostream& output, const transport_catalogue::Trasport_catalogue& trc);

//��������� ������
RequestType Request_type(std::string_view str);

//������ - ���������.
void Request_Stop(std::string_view line, std::ostream& output, const transport_catalogue::Trasport_catalogue& trc);

}//stat_reader