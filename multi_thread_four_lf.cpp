#include "multi_thread_four_lf.h"

#include <optional>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <algorithm>
#include <atomic>
#include <iostream>
#include <memory>

using namespace std;

namespace {
template<typename T>
class ThreadSafeLifo {
public:
  void Push(T&& value) {
    auto* node = new Node{std::move(value), head_.load()};
    while (!head_.compare_exchange_strong(node->next, node)) {
    }
  }

  std::optional<T> Pop() {
    Node* cur_head = head_.load();
    while (cur_head && !head_.compare_exchange_strong(cur_head, cur_head->next)) {
    }
    if (cur_head) {
      dispose_.template emplace_back(cur_head);
      return std::move(cur_head->value);
    }
    return std::nullopt;
  }

private:
  struct Node {
    T value;
    Node* next;
  };
  std::atomic<Node*> head_{nullptr};
  vector<unique_ptr<Node>> dispose_;
};

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
}

void CalculateValuesMtLockFree(std::deque<Node>& graph, int thread_count) {
  ThreadSafeLifo<Node*> wait_for_process;
  std::atomic<int> nodes_left = count_if(begin(graph), end(graph), [](const Node& node) {
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

  ThreadExecutor executor{thread_count, [&] {
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