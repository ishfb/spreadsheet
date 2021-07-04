#include <benchmark/benchmark.h>

#include "graph.h"
#include "multi_thread_one.h"
#include "single_thread.h"

#include <random>

using namespace std;

deque<Node> BuildRandomGraph(size_t node_count) {
  default_random_engine rng(node_count);
  size_t arc_count = node_count * uniform_int_distribution<size_t>(2, 10)(rng);

  deque<Node> result;
  for (size_t i = 0; i < node_count; ++i) {
    result.emplace_back(to_string(i));
  }

  uniform_int_distribution<size_t> node_index(0, node_count - 1);
  for (size_t i = 0; i < arc_count; ++i) {
    size_t first = node_index(rng);
    size_t second = node_index(rng);
    if (first > second) {
      swap(first, second);
    }
    result[first].AddDependancy(&result[second]);
    result[second].AddDependentNode(&result[first]);
  }

  uniform_int_distribution<int> value(-1000, 1000);
  for (auto& node : result) {
    if (node.DependencyNodes().empty()) {
      node.SetValue(value(rng));
    }
  }

  return result;
}

static void BM_SingleThread(benchmark::State& state) {
  size_t node_count = state.range(0);

  vector<deque<Node>> graphs(5000);
  for (auto& g : graphs) {
    g = BuildRandomGraph(node_count);
  }

  auto cur_graph = graphs.begin();
  for (auto _ : state) {
    if (cur_graph == graphs.end()) {
      std::terminate();
    }
    CalculateValuesST(*cur_graph);
    ++cur_graph;
  }
}
// Register the function as a benchmark
BENCHMARK(BM_SingleThread)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_MultiThreadOne(benchmark::State& state) {
  size_t node_count = state.range(0);

  vector<deque<Node>> graphs(5000);
  for (auto& g : graphs) {
    g = BuildRandomGraph(node_count);
  }

  auto cur_graph = graphs.begin();
  for (auto _ : state) {
    if (cur_graph == graphs.end()) {
      std::terminate();
    }
    CalculateValuesMT(*cur_graph);
    ++cur_graph;
  }
}
// Register the function as a benchmark
BENCHMARK(BM_MultiThreadOne)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();