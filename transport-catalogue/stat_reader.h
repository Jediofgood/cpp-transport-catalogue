#pragma once
//чтение запросов на вывод и сам вывод;

#include <string>
#include <iostream>
#include <sstream>

#include "transport_catalogue.h"

namespace stat_reader{

enum class RequestType {
    Bus,
    Stop,
};

//Начала обращений к БД. 
void db_request(std::istream& input, std::ostream& output, const transport_catalogue::Trasport_catalogue& trc);

//Запрос - Автобус
void Request_Bus(std::string_view line, std::ostream& output, const transport_catalogue::Trasport_catalogue& trc);

//определям запрос
RequestType Request_type(std::string_view str);

//Запрос - Остановка.
void Request_Stop(std::string_view line, std::ostream& output, const transport_catalogue::Trasport_catalogue& trc);

}//stat_reader