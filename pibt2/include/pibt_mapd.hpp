#pragma once
#include "agent.hpp"
#include "assignment.hpp"
#include "solver.hpp"

// struct Agent {
//   int id;
//   Node* v_now;        // current location
//   Node* v_next;       // next location
//   Node* g;            // goal
//   int elapsed;        // eta
//   float tie_breaker;  // epsilon, tie-breaker
//   Task* task;
//   Task* target_task;
// };
class PIBT_MAPD : public MAPD_Solver
{
public:
  static const std::string SOLVER_NAME;

private:
  // PIBT agent

  using Agents = std::vector<Agent*>;

  // <node-id, agent>, whether the node is occupied or not
  // work as reservation table
  Agents occupied_now;
  Agents occupied_next;

  // result of priority inheritance: true -> valid, false -> invalid
  bool funcPIBT(Agent* ai, Agent* aj = nullptr);
  void clearMemory(Agents& A);

  void initializeAssignmentHeaps(Agents& A);
  void initializeAssignments(Agents& A);
  void buildAssignmentHeaps(Agents& A, Tasks& unassigned);
  // main
  void run();

public:
  PIBT_MAPD(MAPD_Instance* _P, bool _use_distance_table = false);
  ~PIBT_MAPD() {}

  static void printHelp();
};
