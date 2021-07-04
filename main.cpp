#include <atomic>
#include <algorithm>
#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <string_view>
#include <string>
#include <optional>
#include <deque>
#include <sstream>
#include <queue>

using namespace std;
using namespace std::string_literals;
using namespace std::string_view_literals;

class InputParser {
public:
  InputParser(int argc, char* argv[])
      : mode(argc > 1 ? argv[1] : "st"),
        input_holder(argc > 2 && argv[2] != "-"sv ? new ifstream(argv[2]) : nullptr),
        output_holder(argc > 3 && argv[3] != "-"sv ? new ofstream(argv[3]) : nullptr),
        input(input_holder ? *input_holder : cin),
        output(output_holder ? *output_holder : cout) {
  }

  istream& GetInputStream() const {
    return input;
  }

  ostream& GetOutputStream() const {
    return output;
  }

  const string& GetMode() const {
    return mode;
  }

private:
  string mode;
  unique_ptr<istream> input_holder;
  unique_ptr<ostream> output_holder;
  istream& input;
  ostream& output;
};

class Node {
public:
  explicit Node(string name) : name_(std::move(name)) {}

  bool HasValue() const { return value_.has_value(); }
  void SetValue(int64_t value) { value_ = value; }

  void AddDependancy(Node* dest) {
    dependecies_.push_back(dest);
    ++wait_for_dependecies_count;
  }

  void AddDependentNode(Node* node) {
    dependent_.push_back(node);
  }

  const vector<Node*>& DependentNodes() const { return dependent_; }
  const vector<Node*>& DependencyNodes() const { return dependecies_; }

  const string& Name() const { return name_; }
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
  string name_;
  optional<int64_t> value_;
  vector<Node*> dependecies_;
  vector<Node*> dependent_;

  atomic<int> wait_for_dependecies_count = 0;
};

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

int64_t CalculateNodeValue(const Node& cur) {
  if (cur.HasValue()) {
    ostringstream msg;
    msg << "Node " << cur.Name() << " unexpectedly has value " << cur.Value();
    throw runtime_error(msg.str());
  }
  if (cur.DependencyNodes().empty()) {
    ostringstream msg;
    msg << "Node " << cur.Name() << " depends on nothing but has no value";
    throw runtime_error(msg.str());
  }

  int64_t value = 0;
  for (const Node* n : cur.DependencyNodes()) {
    if (!n->HasValue()) {
      ostringstream msg;
      msg << "Node " << cur.Name() << " depends on " << n->Name()
          << " which is still not computed";
      throw runtime_error(msg.str());
    }
    value += n->Value();
  }
  return value;
}

template<typename T>
class ThreadSafeQueue {
public:
  void Push(T&& value) {
    lock_guard<mutex> lg(m_);
    queue_.push(std::move(value));
  }

  optional<T> Pop() {
    lock_guard<mutex> lg(m_);
    if (queue_.empty()) {
      return std::nullopt;
    }
    T result = std::move(queue_.front());
    queue_.pop();
    return result;
  }

private:
  mutex m_;
  queue<T> queue_;
};

void CalculateValuesST(deque<Node>& graph) {
  queue<Node*> wait_for_process;

  auto add_to_queue = [&wait_for_process](Node& node) {
    wait_for_process.push(&node);
  };

  for (Node& n : graph) {
    if (n.HasValue()) {
      n.SignalReady(add_to_queue);
    }
  }

  while (!wait_for_process.empty()) {
    Node* cur = wait_for_process.front();
    wait_for_process.pop();

    cur->SetValue(CalculateNodeValue(*cur));
    cur->SignalReady(add_to_queue);
  }
}

class ThreadExecutor {
public:
  ThreadExecutor() = default;

  //Starts multiple threads with the same task
  template<typename... Args>
  ThreadExecutor(int threadCount, Args&& ... args) {
    RunMultiple(threadCount, std::forward<Args>(args)...);
  }

  //Starts a thread
  template<typename... Args>
  void Run(Args&& ... args) {
    threads.emplace_back(std::forward<Args>(args)...);
  }

  //Starts multiple threads with the same task
  template<typename... Args>
  void RunMultiple(int threadCount, Args&& ... args) {
    for (int i = 0; i < threadCount; ++i) {
      Run(std::forward<Args>(args)...);
    }
  }

  ~ThreadExecutor() {
    for (auto& thread : threads) {
      thread.join();
    }
  }
private:
  std::vector<std::thread> threads;
};

void CalculateValuesMT(deque<Node>& graph) {
  ThreadSafeQueue<Node*> wait_for_process;
  atomic<int> nodes_left = count_if(begin(graph), end(graph), [](const Node& node) {
    return !node.HasValue();
  });

  auto add_to_queue = [&wait_for_process](Node& node) {
    wait_for_process.Push(&node);
  };

  for (Node& n : graph) {
    if (n.HasValue()) {
      n.SignalReady(add_to_queue);
    }
  }

  ThreadExecutor executor{4, [&] {
    while (nodes_left > 0) {
      if (auto item = wait_for_process.Pop(); item) {
        Node* node = item.value();
        node->SetValue(CalculateNodeValue(*node));
        --nodes_left;
        node->SignalReady(add_to_queue);
      }
    }
  }};
}

void DebugPrintGraph(const deque<Node>& graph, ostream& output) {
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
