#pragma once
#include <queue>

#include "pibt_mapd.hpp"
#include "problem.hpp"
#include "task.hpp"

class TaskAssignment
{
public:
  TaskAssignment();

  int current_timestep = -1;
  int current_batch_index = -1;

  struct CompareAssignment {
    TaskAssignment* instance;
    CompareAssignment(TaskAssignment* instance) : instance(instance){};
    bool operator()(TaskAssignment* t1, TaskAssignment* t2)
    {
      // Current service times
      int t1_st = instance->current_timestep - t1->task->timestep_appear;
      if (t1_st < 0) t1_st = 0;
      int t2_st = instance->current_timestep - t2->task->timestep_appear;
      if (t2_st < 0) t2_st = 0;

      // Batch index order error BLE
      int t1_ble = instance->current_batch_index - t1->task->batch_id;
      int t2_ble = instance->current_batch_index - t2->task->batch_id;
      if (t1_ble < 0) t1_ble = 0;
      if (t2_ble < 0) t2_ble = 0;

      // Batch order weighted error
      int t1_bowe = (t1_ble + 1) * t1_st;
      int t2_bowe = (t2_ble + 1) * t2_st;
      if (t1->task->batch_id == t2->task->batch_id) {
        // Same batch
        return false;
      } else {
        return t1_bowe < t2_bowe;
      }
    };
  };

  int getCost() { return cost; };
  int setCost(int c) { cost = c; };

private:
  Task* task;
  Agent* agent;
  int agent_id;
  int cost;
  int distance;
  // std::priority_queue<Task*, Tasks, CompareAssignment> pq_assignments;

  // std::priority_queue<Task*, Tasks, CompareAssignment> getHeap()
  // {
  //   return pq_assignments;
  // }
};

class TaskAssignments
{
public:
  TaskAssignments();

  struct CompareAssignments {
    bool operator()(TaskAssignments* a, const TaskAssignments* b)
    {
      // Compare each top
      auto pq1 = a->getHeap();
    }
  };

  std::priority_queue<TaskAssignment*, std::vector<TaskAssignment*>,
                      CompareAssignments>
  getHeap()
  {
    return pq_assignments;
  }

private:
  MAPD_Instance* _instance;
  std::priority_queue<TaskAssignment*, std::vector<TaskAssignment*>,
                      CompareAssignments>
      pq_assignments;
};