#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <string>
#include <string_view>

using namespace std;

class InputParser {
public:
  InputParser(int argc, char *argv[])
      : input_holder(argc > 1 && string_view(argv[1]) != "-" ? new ifstream(argv[1]) : nullptr),
        output_holder(argc > 2 && string_view(argv[2]) != "-" ? new ofstream(argv[2]) : nullptr),
        input(input_holder ? *input_holder : cin), output(output_holder ? *output_holder : cout) {
  }

  istream &GetInputStream() const {
    return input;
  }

  ostream &GetOutputStream() const {
    return output;
  }

private:
  unique_ptr<istream> input_holder;
  unique_ptr<ostream> output_holder;
  istream &input;
  ostream &output;
};

class Node {
public:
  string_view Name() const {}
  int64_t Value() const {}
};

class Graph {
public:
  const vector<Node> &Nodes() const {}
};

Graph ReadGraph(istream &input) {

}

void CalculateValues(Graph &graph) {}

int main(int argc, char *argv[]) {
  InputParser input_parser(argc, argv);

  string s;
  input_parser.GetInputStream() >> s;
  input_parser.GetOutputStream() << s;

//  Graph graph = ReadGraph(input_parser.GetInputStream());
//  CalculateValues(graph);
//  for (const Node& node : graph.Nodes()) {
//    input_parser.GetOutputStream() << node.Name() << " = " << node.Value() << '\n';
//  }

  return 0;
}
