#include "input_parser.h"
#include <queue>
#include <sstream>
#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <atomic>
#include "graph.h"

void DebugPrintGraph(const std::deque<Node>& graph, std::ostream& output) {
  for (const Node& node : graph) {
    output << node.Name() << ": ";
    if (node.HasValue()) {
      output << node.Value();
    } else {
      output << "[X]";
    }
    output << " dependent";
    for (auto* n : node.DependentNodes()) {
      output << ' ' << n->Name();
    }
    output << '\n';
  }
}

Node::Node(std::string name) : name_(std::move(name)) {}

void Node::AddDependancy(Node* dest) {
  dependecies_.push_back(dest);
  ++wait_for_dependecies_count;
}

void Node::AddDependentNode(Node* node) {
  dependent_.push_back(node);
}

int64_t CalculateNodeValue(const Node& cur) {
  std::chrono::microseconds delay{std::hash<std::__cxx11::string>()(cur.Name()) % 20};
  std::this_thread::sleep_for(delay);

  if (cur.HasValue()) {
    std::ostringstream msg;
    msg << "Node " << cur.Name() << " unexpectedly has value " << cur.Value();
    throw std::runtime_error(msg.str());
  }
  if (cur.DependencyNodes().empty()) {
    std::ostringstream msg;
    msg << "Node " << cur.Name() << " depends on nothing but has no value";
    throw std::runtime_error(msg.str());
  }

  int64_t value = 0;
  for (const Node* n : cur.DependencyNodes()) {
    if (!n->HasValue()) {
      std::ostringstream msg;
      msg << "Node " << cur.Name() << " depends on " << n->Name()
          << " which is still not computed";
      throw std::runtime_error(msg.str());
    }
    value += n->Value();
  }
  return value;
}