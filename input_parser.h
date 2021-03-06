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

  int GetThreadCount() const { return thread_count; }

private:
  std::string mode;
  std::unique_ptr<std::istream> input_holder;
  std::unique_ptr<std::ostream> output_holder;
  int thread_count;
  std::istream& input;
  std::ostream& output;
};