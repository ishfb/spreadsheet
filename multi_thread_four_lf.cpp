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
#include "concurrentqueue.h"

using namespace std;

namespace {

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
  ::moodycamel::ConcurrentQueue<Node*> wait_for_process;
  std::atomic<int> nodes_left = count_if(begin(graph), end(graph), [](const Node& node) {
    return !node.HasValue();
  });

  for (Node& n : graph) {
    if (n.HasValue()) {
      n.SignalReady([&wait_for_process](Node& node) {
        wait_for_process.enqueue(&node);
      });
    }
  }

  ThreadExecutor executor{thread_count, [&] {
    moodycamel::ProducerToken prod_token(wait_for_process);
    moodycamel::ConsumerToken cons_token(wait_for_process);
    std::vector<Node*> nodes_to_process;

    while (nodes_left > 0) {
      Node* node = nullptr;
      for (size_t i = 0; i < 10 && !wait_for_process.try_dequeue(cons_token, node); ++i) {
        std::this_thread::sleep_for(std::chrono::microseconds(1 << i));
      }

      while (node) {
        node->SetValue(CalculateNodeValue(*node));
        --nodes_left;
        node->SignalReady([&](Node& ready_to_be_processed) {
          nodes_to_process.push_back(&ready_to_be_processed);
        });
        if (nodes_to_process.size() > 1) {
          wait_for_process.enqueue_bulk(prod_token, nodes_to_process.begin() + 1,
                                        nodes_to_process.size() - 1);
        }
        node = nodes_to_process.empty() ? nullptr : nodes_to_process.front();
        nodes_to_process.clear();
      }
    }
  }};
}