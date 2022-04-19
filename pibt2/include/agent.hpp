#pragma once
#include <graph.hpp>

#include "task.hpp"

struct Agent {
  int id;
  Node* v_now;        // current location
  Node* v_next;       // next location
  Node* g;            // goal
  int elapsed;        // eta
  float tie_breaker;  // epsilon, tie-breaker
  Task* task;
  Task* target_task;
};