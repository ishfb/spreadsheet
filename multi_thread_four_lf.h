#pragma once

#include "graph.h"

void CalculateValuesMtLockFree(std::deque<Node>& graph, int thread_count);