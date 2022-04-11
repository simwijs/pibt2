#pragma once
#include <cassert>
#include <graph.hpp>
#include <queue>
#include <random>

#include "default_params.hpp"
#include "task.hpp"
#include "util.hpp"
using Config = std::vector<Node*>;  // < loc_0[t], loc_1[t], ... >
using Configs = std::vector<Config>;

using pq = std::priority_queue<Task>;

// check two configurations are same or not
[[maybe_unused]] static bool sameConfig(const Config& config_i,
                                        const Config& config_j)
{
  if (config_i.size() != config_j.size()) return false;
  const int size_i = config_i.size();
  for (int k = 0; k < size_i; ++k) {
    if (config_i[k] != config_j[k]) return false;
  }
  return true;
}

[[maybe_unused]] static int getPathCost(const Path& path)
{
  int cost = path.size() - 1;
  auto itr = path.end() - 1;
  while (itr != path.begin() && *itr == *(itr - 1)) {
    --cost;
    --itr;
  }
  return cost;
}

class Problem
{
protected:
  std::string instance;  // instance name
  Graph* G;              // graph
  std::mt19937* MT;      // seed
  Config config_s;       // initial configuration
  Config config_g;       // goal configuration
  int num_agents;        // number of agents
  int max_timestep;      // timestep limit
  int max_comp_time;     // comp_time limit, ms

  // utilities
  void halt(const std::string& msg) const;
  void warn(const std::string& msg) const;

public:
  Problem(){};
  Problem(const std::string& _instance) : instance(_instance) {}
  Problem(std::string _instance, Graph* _G, std::mt19937* _MT, Config _config_s,
          Config _config_g, int _num_agents, int _max_timestep,
          int _max_comp_time);
  ~Problem(){};

  Graph* getG() { return G; }
  int getNum() { return num_agents; }
  std::mt19937* getMT() { return MT; }
  Node* getStart(int i) const;  // return start of a_i
  Node* getGoal(int i) const;   // return  goal of a_i
  Config getConfigStart() const { return config_s; };
  Config getConfigGoal() const { return config_g; };
  int getMaxTimestep() { return max_timestep; };
  int getMaxCompTime() { return max_comp_time; };
  std::string getInstanceFileName() { return instance; };

  void setMaxCompTime(const int t) { max_comp_time = t; }
};

class MAPF_Instance : public Problem
{
private:
  const bool instance_initialized;  // for memory manage

  // set starts and goals randomly
  void setRandomStartsGoals();

  // set well-formed instance
  void setWellFormedInstance();

public:
  MAPF_Instance(const std::string& _instance);
  MAPF_Instance(MAPF_Instance* P, Config _config_s, Config _config_g,
                int _max_comp_time, int _max_timestep);
  MAPF_Instance(MAPF_Instance* P, int _max_comp_time);
  ~MAPF_Instance();

  bool isInitializedInstance() const { return instance_initialized; }

  // used when making new instance file
  void makeScenFile(const std::string& output_file);
};

class MAPD_Instance : public Problem
{
public:
  MAPD_Instance(const std::string& _task_file,                    // Simon #6
                const std::string& _map_file,                     // #11
                const bool _is_batched, const bool _batch_prio);  // #11
  ~MAPD_Instance();
  bool batch_prio;
  void update();
  int getCurrentTimestep() const { return current_timestep; }
  float getTaskFrequency() const { return task_frequency; }
  float getTaskNum() const { return task_num; }
  std::string getTaskName() const { return task_file; }
  Tasks getOpenTasks() { return TASKS_OPEN; }
  Tasks getClosedTasks() { return TASKS_CLOSED; }
  Nodes getEndpoints() { return LOCATIONS; }
  Batches getBatches() { return batches; }

  struct CompareTask {
    MAPD_Instance* instance;
    CompareTask(MAPD_Instance* instance) : instance(instance) {}
    bool operator()(Task* t1, Task* t2)
    {
      // Current service times
      int t1_st = instance->current_timestep - t1->timestep_appear;
      if (t1_st < 0) t1_st = 0;
      int t2_st = instance->current_timestep - t2->timestep_appear;
      if (t2_st < 0) t2_st = 0;

      // Batch index order error BLE
      int t1_ble = instance->current_batch_index - t1->batch_id;
      int t2_ble = instance->current_batch_index - t2->batch_id;
      if (t1_ble < 0) t1_ble = 0;
      if (t2_ble < 0) t2_ble = 0;

      // Batch order weighted error
      int t1_bowe = (t1_ble + 1) * t1_st;
      int t2_bowe = (t2_ble + 1) * t2_st;

      if (t1->batch_id == t2->batch_id) {
        // Same batch
        return false;
      } else {
        return t1_bowe < t2_bowe;
      }
    };
  };
  std::priority_queue<Task*, Tasks, CompareTask> getHeap() { return pq_tasks; }

private:
  float task_frequency;
  int task_num;
  // Simon #6
  std::string task_file;
  std::string map_file;
  bool is_batched;

  int current_timestep;         // current timestep
  int current_batch_index = 0;  // Increased whenever a batch is finished
  Tasks TASKS_OPEN;
  Tasks TASKS_CLOSED;
  Batches batches;
  std::vector<int> finished_batches;
  // Simon #6
  TimedTasks TASKS_SCHEDULED;  // Tasks indexed by timestep
  Tasks TASKS;                 // All tasks

  std::priority_queue<Task*, Tasks,
                      CompareTask>
      pq_tasks;  // Priority queue for choosing task

  Nodes LOCS_PICKUP;             // candidates of pickup locations
  Nodes LOCS_DELIVERY;           // candidates of delivery locations
  Nodes LOCS_NONTASK_ENDPOINTS;  // endpoints, not necessary for PIBT
  Nodes LOCATIONS;               // pickup, delivery, nontasks

  bool specify_pickup_deliv_locs;
  void setupSpetialNodes();
  void read_task_file(bool is_batched);
  void read_map_file();
  Node* get_endpoint_node_from_int(int num);
};
