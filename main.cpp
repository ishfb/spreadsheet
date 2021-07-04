#include <iostream>
#include <sstream>
#include <unordered_map>

#include "input_parser.h"
#include "graph.h"
#include "single_thread.h"
#include "multi_thread_one.h"

using namespace std;

deque<Node> ReadGraph(istream& input) {
  deque<Node> nodes;
  unordered_map<string, Node*> known_nodes;

  auto get_node = [&nodes, &known_nodes](const string& name) {
    if (auto it = known_nodes.find(name); it == known_nodes.end()) {
      auto* node_ptr = &nodes.emplace_back(name);
      known_nodes[name] = node_ptr;
      return node_ptr;
    } else {
      return it->second;
    }
  };

  vector<string> children;
  for (string line; getline(input, line);) {
    istringstream line_input(line);
    string node_name;
    line_input >> node_name;
    Node* parent = get_node(node_name);

    line_input >> ws;
    line_input.ignore();
    line_input >> ws;

    children.clear();
    for (string value; getline(line_input, value, '+');) {
      children.push_back(std::move(value));
    }

    if (children.empty()) {
      ostringstream msg;
      msg << "No value for node " << parent->Name() << ". Line is " << line;
      throw invalid_argument(msg.str());
    } else if (children.size() == 1 && isdigit(children[0][0])) {
      istringstream is(children[0]);
      int64_t value;
      is >> value;
      parent->SetValue(value);
    } else {
      for (const auto& value : children) {
        if (isdigit(value[0])) {
          ostringstream msg;
          msg << "Node " << parent->Name() << " has value " << value
              << " in its dependencies";
          throw invalid_argument(msg.str());
        }
        auto* child = get_node(value);
        parent->AddDependancy(child);
        child->AddDependentNode(parent);
      }
    }
  }
  return nodes;
}

int main(int argc, char* argv[]) {
  std::ios_base::sync_with_stdio(false);
  cin.tie(nullptr);

  InputParser input_parser(argc, argv);

  auto graph = ReadGraph(input_parser.GetInputStream());
//  DebugPrintGraph(graph, input_parser.GetOutputStream());
  if (input_parser.GetMode() == "st") {
    CalculateValuesST(graph);
  } else if (input_parser.GetMode() == "mt") {
    CalculateValuesMT(graph);
  } else {
    throw invalid_argument("Unknown mode " + input_parser.GetMode());
  }
  for (const Node& node : graph) {
    input_parser.GetOutputStream() << node.Name() << " = " << node.Value() << '\n';
  }

  return 0;
}
