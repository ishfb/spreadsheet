#include "single_thread.h"

#include <queue>

void CalculateValuesST(std::deque<Node>& graph) {
  std::queue<Node*> wait_for_process;

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