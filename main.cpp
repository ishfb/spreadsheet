#include <iostream>
#include <vector>

using namespace std;

class InputParser {
public:
  InputParser(int argc, char *argv[]) {

  }

  istream& GetInputStream() const {

  }

  ostream& GetOutputStream() const {

  }
};

class Node {
public:
  string_view Name() const {}
  int64_t Value() const {}
};

class Graph {
public:
  const vector<Node>& Nodes() const {}
};

Graph ReadGraph(istream& input) {

}

void CalculateValues(Graph& graph) {}

int main(int argc, char *argv[]) {
  InputParser input_parser(argc, argv);

  Graph graph = ReadGraph(input_parser.GetInputStream());
  CalculateValues(graph);
  for (const Node& node : graph.Nodes()) {
    input_parser.GetOutputStream() << node.Name() << " = " << node.Value() << '\n';
  }

  return 0;
}
