#include <iostream>
#include <istream>
#include <fstream>
#include <string_view>

#include <transport_catalogue.pb.h>

#include <memory>

#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main23(int argc, char* argv[]) {
	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	const std::string_view mode(argv[1]);

	if (mode == "make_base"sv) {

		MakeBase(std::cin);

	}
	else if (mode == "process_requests"sv) {

		ProcessRequests(std::cin);

	}
	else {
		PrintUsage();
		return 1;
	}
}

void main() {
	using namespace std;
	ifstream input1("1input.json");
	ifstream input2("2input.json");
	ofstream output("output.json");

	MakeBase(input1);
	ProcessRequests(input2, output);
}
