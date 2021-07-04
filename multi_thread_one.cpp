#include "multi_thread_one.h"

#include <optional>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <algorithm>
#include <atomic>
#include <iostream>

template<typename T>
class ThreadSafeQueue {
public:
  void Push(T&& value) {
    std::lock_guard<std::mutex> lg(m_);
    queue_.push(std::move(value));
  }

  std::optional<T> Pop() {
    std::lock_guard<std::mutex> lg(m_);
    if (queue_.empty()) {
      return std::nullopt;
    }
    T result = std::move(queue_.front());
    queue_.pop();
    return result;
  }

private:
  std::mutex m_;
  std::queue<T> queue_;
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

void CalculateValuesMT(std::deque<Node>& graph) {
  ThreadSafeQueue<Node*> wait_for_process;
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

  int thread_count = std::thread::hardware_concurrency();
//  std::cerr << "Thread count is " << thread_count << '\n';
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