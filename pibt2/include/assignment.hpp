#pragma once
#include <queue>
#include <set>

#include "agent.hpp"
#include "task.hpp"

class TaskAssignment
{
public:
  TaskAssignment(Task* task, Agent* agent) : task(task), agent(agent) {}

  // int current_timestep = -1;
  // int current_batch_index = -1;

  // struct CompareAssignment {
  //   TaskAssignment* instance;
  //   CompareAssignment(TaskAssignment* instance) : instance(instance){};
  //   bool operator()(TaskAssignment* t1, TaskAssignment* t2)
  //   {
  //     // Current service times
  //     int t1_st = instance->current_timestep - t1->task->timestep_appear;
  //     if (t1_st < 0) t1_st = 0;
  //     int t2_st = instance->current_timestep - t2->task->timestep_appear;
  //     if (t2_st < 0) t2_st = 0;

  //     // Batch index order error BLE
  //     int t1_ble = instance->current_batch_index - t1->task->batch_id;
  //     int t2_ble = instance->current_batch_index - t2->task->batch_id;
  //     if (t1_ble < 0) t1_ble = 0;
  //     if (t2_ble < 0) t2_ble = 0;

  //     // TODO: Use distance somehow, also each agents priority?

  //     // Batch order weighted error
  //     int t1_bowe = (t1_ble + 1) * t1_st;
  //     int t2_bowe = (t2_ble + 1) * t2_st;
  //     if (t1->task->batch_id == t2->task->batch_id) {
  //       // Same batch
  //       return false;
  //     } else {
  //       return t1_bowe < t2_bowe;
  //     }
  //   };
  // };
  Task* task;
  Agent* agent;
  int cost;
  int distance;
};

class TaskAssignments
{
public:
  TaskAssignments()
      : pq_assignments(CompareAssignments(this)),
        set_assignments(CompareAssignments(this)){};

  int current_timestep = -1;
  int current_batch_index = -1;

  struct CompareAssignments {
    TaskAssignments* instance;
    CompareAssignments(TaskAssignments* instance) : instance(instance){};
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

      // TODO: Use distance somehow, also each agents priority?

      // Batch order weighted error
      int t1_bowe = (t1_ble + 1) * t1_st;
      int t2_bowe = (t2_ble + 1) * t2_st;
      if (t1->task->batch_id == t2->task->batch_id) {
        // Same batch
        return false;
      } else {
        int d1 = t1->distance + 1;
        int d2 = t2->distance + 1;
        return t1_bowe / d1 < t2_bowe / d2;
      }
    }
  };

  std::priority_queue<TaskAssignment*, std::vector<TaskAssignment*>,
                      CompareAssignments>&
  getHeap()
  {
    return pq_assignments;
  }

  std::set<TaskAssignment*, CompareAssignments>& getSet()
  {
    return set_assignments;
  }

private:
  // MAPD_Instance* _instance;
  std::priority_queue<TaskAssignment*, std::vector<TaskAssignment*>,
                      CompareAssignments>
      pq_assignments;
  std::set<TaskAssignment*, CompareAssignments> set_assignments;
};