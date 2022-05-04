#pragma once
#include <iostream>
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
  int opt_variant;
  float opt_constant;

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

      // Test the interesting formulas
      // (1-K)*bowe + dist*K, k in [0, 1]
      // bowe*dist
      // before hyperpopt, K in {0.1, 0.5, 0.9} 1 and 0 already tested

      // Batch order weighted error
      float t1_bowe = (t1_ble + 1) * t1_st;
      float t2_bowe = (t2_ble + 1) * t2_st;
      if (t1->task->batch_id == t2->task->batch_id) {
        // Same batch
        return false;
      } else {
        float d1 = t1->distance + 1;
        float d2 = t2->distance + 1;
        float K = instance->opt_constant;
        // Different prioritization formulas, 6 is good
        float prio1;
        float prio2;
        switch (instance->opt_variant) {
          case 1:
            prio1 = t1_bowe / (d1 + K);
            prio2 = t2_bowe / (d2 + K);
            break;
          case 2:
            prio1 = (t1_bowe + K) / d1;
            prio2 = (t2_bowe + K) / d2;
            break;
          case 3:
            prio1 = t1_bowe / (d1 / K + 1);
            prio2 = t2_bowe / (d2 / K + 1);
            break;
          case 4:
            prio1 = (t1_bowe / K) / d1;
            prio2 = (t2_bowe / K) / d2;
            break;
          case 5:
            prio1 = t1_bowe * d1;
            prio2 = t2_bowe * d2;
            break;
          case 6:
            prio1 = (1.0 - K) * t1_bowe + d1 * K;
            prio2 = (1.0 - K) * t2_bowe + d2 * K;
            break;
          case 7:
            prio1 = (1.0 - K) * 100.0 / t1_bowe + d1 * K;
            prio2 = (1.0 - K) * 100.0 / t2_bowe + d2 * K;
            break;
          case 8:
            prio1 = d1 / (t1_bowe + 1);
            prio2 = d2 / (t2_bowe + 1);
            break;
          case 9:
            prio1 = d1 * K - (1 - K) * t1_bowe;
            prio2 = d2 * K - (1 - K) * t2_bowe;
            break;
          case 10:
            // Quite good
            prio1 = ((1 - K) * t1_bowe) / (K * d1 + 1);
            prio2 = ((1 - K) * t2_bowe) / (K * d2 + 1);
            break;
          case 11:
            prio1 = ((1 - K) * t1_bowe + 1) / (K * d1 + 1);
            prio2 = ((1 - K) * t2_bowe + 1) / (K * d2 + 1);
            break;
          case 12:
            prio1 = (K * t1_bowe + 1) / (K + d1);
            prio2 = (K * t2_bowe + 1) / (K + d2);
            break;
          default:
            return false;
            break;
        }
        // std::cout << "bowe:" << t1_bowe << ", " << t2_bowe << " d:" << d1
        //           << ", " << d2 << " prios: " << prio1 << "<" << prio2
        //           << std::endl;
        return prio1 < prio2;
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