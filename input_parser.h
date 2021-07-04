#pragma once

#include <string>
#include <istream>
#include <ostream>
#include <memory>

class InputParser {
public:
  InputParser(int argc, char* argv[]);

  std::istream& GetInputStream() const {
    return input;
  }

  std::ostream& GetOutputStream() const {
    return output;
  }

  const std::string& GetMode() const {
    return mode;
  }

private:
  std::string mode;
  std::unique_ptr<std::istream> input_holder;
  std::unique_ptr<std::ostream> output_holder;
  std::istream& input;
  std::ostream& output;
};