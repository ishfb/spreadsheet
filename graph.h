#pragma once

#include <string>
#include <vector>
#include <optional>
#include <atomic>
#include <deque>
#include <ostream>

class Node {
public:
  explicit Node(std::string name);

  bool HasValue() const { return value_.has_value(); }
  void SetValue(int64_t value) { value_ = value; }

  void AddDependancy(Node* dest);
  void AddDependentNode(Node* node);

  const std::vector<Node*>& DependentNodes() const { return dependent_; }
  const std::vector<Node*>& DependencyNodes() const { return dependecies_; }

  const std::string& Name() const { return name_; }
  int64_t Value() const { return *value_; }

  template<typename Callback>
  void SignalReady(Callback callback) {
    for (Node* d : dependent_) {
      if (int v = --d->wait_for_dependecies_count; v == 0) {
        callback(*d);
      } else if (v < 0) {
        throw std::runtime_error("SignalReady got negative value for node " + d->Name());
      }
    }
  }

private:
  std::string name_;
  std::optional<int64_t> value_;
  std::vector<Node*> dependecies_;
  std::vector<Node*> dependent_;

  std::atomic<int> wait_for_dependecies_count = 0;
};

int64_t CalculateNodeValue(const Node& cur);

void DebugPrintGraph(const std::deque<Node>& graph, std::ostream& output);
