#include <benchmark/benchmark.h>

#include "graph.h"
#include "multi_thread_one.h"
#include "multi_thread_two_batches.h"
#include "multi_thread_three_work_stealing_queue.h"
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

template<typename TestFunc>
void Run(benchmark::State& state, size_t node_count, TestFunc test_func) {
  auto graph = BuildRandomGraph(node_count);

  for (auto _ : state) {
    for (auto& node : graph) {
      node.Reset();
    }
    test_func(graph);
  }
}

static void BM_SingleThread(benchmark::State& state) {
  Run(state, state.range(0), CalculateValuesST);
}

// Register the function as a benchmark
//BENCHMARK(BM_SingleThread)->RangeMultiplier(100)->Range(100, 1'000'000);

static void BM_MultiThreadOne(benchmark::State& state) {
  Run(state, state.range(0), CalculateValuesMT);
}
// Register the function as a benchmark
//BENCHMARK(BM_MultiThreadOne)->Arg(100)->Arg(1000)->Arg(10000)->Arg(1'000'000);

static void BM_MultiThreadTwo(benchmark::State& state) {
  Run(state, state.range(1),
      [tc = state.range(0)](auto&& graph) { CalculateValuesMtBatches(graph, tc); });
}
// Register the function as a benchmark
//BENCHMARK(BM_MultiThreadTwo)->Args({128, 1'000'000});
BENCHMARK(BM_MultiThreadTwo)
->ArgsProduct({{64, 96, 128, 256, 320},
               benchmark::CreateRange(10'000, 1'000'000, 100),
               });
//BENCHMARK(BM_MultiThreadTwo)
//->ArgsProduct({{64, 96, 128, 160, 192},
//               {1'000'000},
//              });

static void BM_MultiThreadThree(benchmark::State& state) {
  Run(state, state.range(1),
      [tc = state.range(0)](auto&& graph) { CalculateValuesMtWorkStealing(graph, tc); });
}
// Register the function as a benchmark
//BENCHMARK(BM_MultiThreadTwo)->Args({128, 1'000'000});
BENCHMARK(BM_MultiThreadThree)
->ArgsProduct({{64, 96, 128, 256, 320},
               benchmark::CreateRange(10'000, 1'000'000, 100),
               });
//BENCHMARK(BM_MultiThreadTwo)
//->ArgsProduct({{64, 96, 128, 160, 192},
//               {1'000'000},
//              });

BENCHMARK_MAIN();