#include "multi_thread_one.h"

#include <optional>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <algorithm>
#include <atomic>

namespace {
template<typename T>
class ThreadSafeQueue {
public:
  void Push(T&& value) {
    std::lock_guard<std::mutex> lg(m_);
    queue_.push(std::move(value));
  }

  template<typename It>
  void PushMany(It begin, It end) {
    std::lock_guard<std::mutex> lg(m_);
    for (auto i = begin; i != end; ++i) {
      queue_.push(std::move(*i));
    }
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

}

void CalculateValuesMtBatches(std::deque<Node>& graph, int thread_count) {
  ThreadSafeQueue<Node*> wait_for_process;
  std::atomic<int> nodes_left = count_if(begin(graph), end(graph), [](const Node& node) {
    return !node.HasValue();
  });

  for (Node& n : graph) {
    if (n.HasValue()) {
      n.SignalReady([&wait_for_process](Node& node) {
        wait_for_process.Push(&node);
      });
    }
  }

  ThreadExecutor executor{thread_count, [&] {
    std::vector<Node*> nodes_to_process;

    while (nodes_left > 0) {
      if (auto item = wait_for_process.Pop(); item) {
        Node* node = item.value();

        while (node) {
          node->SetValue(CalculateNodeValue(*node));
          --nodes_left;
          node->SignalReady([&](Node& ready_to_be_processed) {
            nodes_to_process.push_back(&ready_to_be_processed);
          });

          if (nodes_to_process.size() > 1) {
            wait_for_process.PushMany(nodes_to_process.begin() + 1,
                                      nodes_to_process.end());
          }
          node = nodes_to_process.empty() ? nullptr : nodes_to_process.front();
          nodes_to_process.clear();
        }
      }
    }
  }};
}