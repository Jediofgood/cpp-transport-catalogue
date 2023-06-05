#pragma once
//чтение запросов на заполнение базы;

#include <sstream>
#include <string>
#include <vector>
//#include <unordered_map>

#include "transport_catalogue.h"
#include "geo.h"

namespace input_readed{

//Класс для опредления запроса
enum class RequestType {
    Stop,
    Bus,
};

class Length_upto {
public:
	std::string_view name_;
	double lenght_;
};

namespace small_part_processing{

//разбиваем для true длины.
std::vector<Length_upto> String_Split_Lenght(std::string_view strv);

//Получить координты из строки
geo::Coordinates Coordinate(std::string line);

//Обработка типа запроса
RequestType Request_type(std::string_view request);

//Разбиваем линию на остановки, убирая лишнии > - и пробелы.
std::vector<std::string_view> SplitIntoWords(std::string_view str);

}//small_part_processing

namespace string_line_processing {
//Сырая информация для БД
std::vector<std::string> Request_lines(std::istream& input);

//Обработка строки - запроса с остановокой
transport_catalogue::Stops Stop_processing(std::string& line);
} //string_line_processing

//Запуск и заполнение БД - результат: готовая БД
transport_catalogue::Trasport_catalogue Start_database(std::istream& input);

}//input_readed

