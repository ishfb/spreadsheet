#include "input_parser.h"

#include <iostream>
#include <fstream>
#include <string_view>

using namespace std;
using namespace std::string_view_literals;

InputParser::InputParser(int argc, char** argv)
    : mode(argc > 1 ? argv[1] : "st"),
      input_holder(argc > 2 && argv[2] != "-"sv ? new ifstream(argv[2]) : nullptr),
      output_holder(argc > 3 && argv[3] != "-"sv ? new ofstream(argv[3]) : nullptr),
      input(input_holder ? *input_holder : std::cin),
      output(output_holder ? *output_holder : std::cout) {
}
