#include <benchmark/benchmark.h>

#include "graph.h"
#include "multi_thread_one.h"
#include "single_thread.h"

#include <random>
#include <iostream>

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
    size_t first, second;
    do {
      first = node_index(rng);
      second = node_index(rng);
    } while (first == second);
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
  auto graph = BuildRandomGraph(node_count);

  for (auto _ : state) {
    for (auto& node : graph) {
      node.Reset();
    }
    CalculateValuesST(graph);
  }
}
// Register the function as a benchmark
BENCHMARK(BM_SingleThread)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_MultiThreadOne(benchmark::State& state) {
  size_t node_count = state.range(0);
  auto graph = BuildRandomGraph(node_count);

  for (auto _ : state) {
    for (auto& node : graph) {
      node.Reset();
    }
    CalculateValuesMT(graph);
  }
}
// Register the function as a benchmark
BENCHMARK(BM_MultiThreadOne)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();