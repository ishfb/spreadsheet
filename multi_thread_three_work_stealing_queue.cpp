#include "multi_thread_three_work_stealing_queue.h"

#include <optional>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <algorithm>
#include <atomic>
#include <memory>
#include <random>

using namespace std;

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

template<typename T>
class WorkStealingQueue {
private:
  using ThreadQueues = vector<unique_ptr<ThreadSafeQueue<T>>>;

  static ThreadQueues BuildQueues(size_t thread_count) {
    ThreadQueues result(thread_count);
    for (auto& item : result) {
      item = make_unique<ThreadSafeQueue<T>>();
    }
    return result;
  }
public:
  explicit WorkStealingQueue(size_t thread_count)
      : thread_count_(thread_count),
        thread_queues_(BuildQueues(thread_count_)) {}

  void Push(size_t id, T value) {
    thread_queues_.at(id)->Push(std::move(value));
  }
  template<typename It>
  void PushMany(size_t id, It begin, It end) {
    thread_queues_.at(id)->template PushMany(begin, end);
  }
  std::optional<T> Pop(size_t id) {
    std::default_random_engine rng{id};
    std::uniform_int_distribution<size_t> next_id(0, thread_count_ - 1);
    for (size_t i = 0; i < thread_count_; ++i) {
      if (auto item = thread_queues_.at(id)->Pop(); item) {
        return item;
      }
      id = next_id(rng);
    }
    return std::nullopt;
  }

private:
  size_t thread_count_;
  const ThreadQueues thread_queues_;
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

void CalculateValuesMtWorkStealing(std::deque<Node>& graph, int thread_count) {
  std::atomic<int> nodes_left = count_if(begin(graph), end(graph), [](const Node& node) {
    return !node.HasValue();
  });

  std::vector<Node*> ready_for_processing;
  for (Node& n : graph) {
    if (n.HasValue()) {
      n.SignalReady([&ready_for_processing](Node& node) {
        ready_for_processing.push_back(&node);
      });
    }
  }

  WorkStealingQueue<Node*> wait_for_process(thread_count);
  for (size_t i = 0; i < ready_for_processing.size(); ++i) {
    wait_for_process.Push(i % thread_count, ready_for_processing[i]);
  }

  std::atomic<size_t> thread_id = 0;
  ThreadExecutor executor{thread_count, [&] {
    size_t my_id = thread_id++;

    std::vector<Node*> nodes_to_process;
//    return;

    while (nodes_left > 0) {
      if (auto item = wait_for_process.Pop(my_id); item) {
        Node* node = item.value();

        while (node) {
          node->SetValue(CalculateNodeValue(*node));
          --nodes_left;
          node->SignalReady([&](Node& ready_to_be_processed) {
            nodes_to_process.push_back(&ready_to_be_processed);
          });

          if (nodes_to_process.size() > 1) {
            wait_for_process.PushMany(my_id, nodes_to_process.begin() + 1,
                                      nodes_to_process.end());
          }
          node = nodes_to_process.empty() ? nullptr : nodes_to_process.front();
          nodes_to_process.clear();
        }
      }
    }
  }};
}