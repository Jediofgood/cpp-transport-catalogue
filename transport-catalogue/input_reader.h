#pragma once
//������ �������� �� ���������� ����;

#include <sstream>
#include <string>
#include <vector>
//#include <unordered_map>

#include "transport_catalogue.h"
#include "geo.h"

namespace input_readed{

//����� ��� ���������� �������
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

//��������� ��� true �����.
std::vector<Length_upto> String_Split_Lenght(std::string_view strv);

//�������� ��������� �� ������
geo::Coordinates Coordinate(std::string line);

//��������� ���� �������
RequestType Request_type(std::string_view request);

//��������� ����� �� ���������, ������ ������ > - � �������.
std::vector<std::string_view> SplitIntoWords(std::string_view str);

}//small_part_processing

namespace string_line_processing {
//����� ���������� ��� ��
std::vector<std::string> Request_lines(std::istream& input);

//��������� ������ - ������� � �����������
transport_catalogue::Stops Stop_processing(std::string& line);
} //string_line_processing

//������ � ���������� �� - ���������: ������� ��
transport_catalogue::Trasport_catalogue Start_database(std::istream& input);

}//input_readed

