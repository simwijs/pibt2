#pragma once
#include <graph.hpp>

static int TASK_ID_CNT = 0;

struct Task {
  int id;
  Node* loc_pickup;
  Node* loc_delivery;
  Node* loc_current;
  int timestep_appear;
  int timestep_finished;
  int timestep_started;  // Not the same as appear time, but when the task has
                         // been picked up by an agent
  int batch_id;
  bool assigned;
  bool finished = false;

  static constexpr int NIL = -1;

  Task(Node* loc_p, Node* loc_d, int t, int batch_id)
      : id(TASK_ID_CNT++),
        loc_pickup(loc_p),
        loc_delivery(loc_d),
        loc_current(loc_p),
        timestep_appear(t),
        timestep_finished(NIL),
        batch_id(batch_id),
        timestep_started(-1),
        assigned(false){};

  int get_service_time()
  {
    if (timestep_finished == NIL)
      throw std::runtime_error("Cannot get service time of an unfinished task");

    return timestep_finished - timestep_appear;
  };
};

using Tasks = std::vector<Task*>;
using TimedTasks = std::vector<Tasks>;  // Simon #6

struct Batch {
  int id, timestep_appear, timestep_finished, ble;
  Tasks tasks;

  Batch(int id) : id(id), timestep_appear(-1), timestep_finished(-1){};

  int get_size() { return tasks.size(); };

  void add_task(Task* t)
  {
    if (timestep_appear == -1) {
      timestep_appear = t->timestep_appear;
    }
    tasks.push_back(t);
  }

  int get_service_time()
  {
    if (!is_finished())
      throw std::runtime_error(
          "Cannot get service time when the batch isn't finished");
    int total = 0;
    for (auto t : tasks) {
      total += t->get_service_time();
    }
    return total;
  }

  bool is_finished()
  {
    for (auto t : tasks) {
      if (t->timestep_finished == -1) {
        return false;
      }
    }
    return true;
  }

  void try_finish()
  {
    if (!is_finished()) return;

    for (auto t : tasks) {
      if (t->timestep_finished > timestep_finished) {
        timestep_finished = t->timestep_finished;
      }
    }
  }
};

using Batches = std::vector<Batch*>;
