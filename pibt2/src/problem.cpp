#include "../include/problem.hpp"

#include <fstream>
#include <regex>

#include "../include/util.hpp"

Problem::Problem(std::string _instance, Graph* _G, std::mt19937* _MT,
                 Config _config_s, Config _config_g, int _num_agents,
                 int _max_timestep, int _max_comp_time)
    : instance(_instance),
      G(_G),
      MT(_MT),
      config_s(_config_s),
      config_g(_config_g),
      num_agents(_num_agents),
      max_timestep(_max_timestep),
      max_comp_time(_max_comp_time){};

Node* Problem::getStart(int i) const
{
  if (!(0 <= i && i < (int)config_s.size())) halt("invalid index");
  return config_s[i];
}

Node* Problem::getGoal(int i) const
{
  if (!(0 <= i && i < (int)config_g.size())) halt("invalid index");
  return config_g[i];
}

void Problem::halt(const std::string& msg) const
{
  std::cout << "error@Problem: " << msg << std::endl;
  this->~Problem();
  std::exit(1);
}

void Problem::warn(const std::string& msg) const
{
  std::cout << "warn@Problem: " << msg << std::endl;
}

// -------------------------------------------
// MAPF

MAPF_Instance::MAPF_Instance(const std::string& _instance)
    : Problem(_instance), instance_initialized(true)
{
  // read instance file
  std::ifstream file(instance);
  if (!file) halt("file " + instance + " is not found.");

  std::string line;
  std::smatch results;
  std::regex r_comment = std::regex(R"(#.+)");
  std::regex r_map = std::regex(R"(map_file=(.+))");
  std::regex r_agents = std::regex(R"(agents=(\d+))");
  std::regex r_seed = std::regex(R"(seed=(\d+))");
  std::regex r_random_problem = std::regex(R"(random_problem=(\d+))");
  std::regex r_well_formed = std::regex(R"(well_formed=(\d+))");
  std::regex r_max_timestep = std::regex(R"(max_timestep=(\d+))");
  std::regex r_max_comp_time = std::regex(R"(max_comp_time=(\d+))");
  std::regex r_sg = std::regex(R"((\d+),(\d+),(\d+),(\d+))");

  bool read_scen = true;
  bool well_formed = false;
  while (getline(file, line)) {
    // for CRLF coding
    if (*(line.end() - 1) == 0x0d) line.pop_back();
    // comment
    if (std::regex_match(line, results, r_comment)) {
      continue;
    }
    // read map
    if (std::regex_match(line, results, r_map)) {
      G = new Grid(results[1].str());
      continue;
    }
    // set agent num
    if (std::regex_match(line, results, r_agents)) {
      num_agents = std::stoi(results[1].str());
      continue;
    }
    // set random seed
    if (std::regex_match(line, results, r_seed)) {
      MT = new std::mt19937(std::stoi(results[1].str()));
      continue;
    }
    // skip reading initial/goal nodes
    if (std::regex_match(line, results, r_random_problem)) {
      if (std::stoi(results[1].str())) {
        read_scen = false;
        config_s.clear();
        config_g.clear();
      }
      continue;
    }
    //
    if (std::regex_match(line, results, r_well_formed)) {
      if (std::stoi(results[1].str())) well_formed = true;
      continue;
    }
    // set max timestep
    if (std::regex_match(line, results, r_max_timestep)) {
      max_timestep = std::stoi(results[1].str());
      continue;
    }
    // set max computation time
    if (std::regex_match(line, results, r_max_comp_time)) {
      max_comp_time = std::stoi(results[1].str());
      continue;
    }
    // read initial/goal nodes
    if (std::regex_match(line, results, r_sg) && read_scen &&
        (int)config_s.size() < num_agents) {
      int x_s = std::stoi(results[1].str());
      int y_s = std::stoi(results[2].str());
      int x_g = std::stoi(results[3].str());
      int y_g = std::stoi(results[4].str());
      if (!G->existNode(x_s, y_s)) {
        halt("start node (" + std::to_string(x_s) + ", " + std::to_string(y_s) +
             ") does not exist, invalid scenario");
      }
      if (!G->existNode(x_g, y_g)) {
        halt("goal node (" + std::to_string(x_g) + ", " + std::to_string(y_g) +
             ") does not exist, invalid scenario");
      }

      Node* s = G->getNode(x_s, y_s);
      Node* g = G->getNode(x_g, y_g);
      config_s.push_back(s);
      config_g.push_back(g);
    }
  }

  // set default value not identified params
  if (MT == nullptr) MT = new std::mt19937(DEFAULT_SEED);
  if (max_timestep == 0) max_timestep = DEFAULT_MAX_TIMESTEP;
  if (max_comp_time == 0) max_comp_time = DEFAULT_MAX_COMP_TIME;

  // check starts/goals
  if (num_agents <= 0) halt("invalid number of agents");
  const int config_s_size = config_s.size();
  if (!config_s.empty() && num_agents > config_s_size) {
    warn("given starts/goals are not sufficient\nrandomly create instances");
  }
  if (num_agents > config_s_size) {
    if (well_formed) {
      setWellFormedInstance();
    } else {
      setRandomStartsGoals();
    }
  }

  // trimming
  config_s.resize(num_agents);
  config_g.resize(num_agents);
}

MAPF_Instance::MAPF_Instance(MAPF_Instance* P, Config _config_s,
                             Config _config_g, int _max_comp_time,
                             int _max_timestep)
    : Problem(P->getInstanceFileName(), P->getG(), P->getMT(), _config_s,
              _config_g, P->getNum(), _max_timestep, _max_comp_time),
      instance_initialized(false)
{
}

MAPF_Instance::MAPF_Instance(MAPF_Instance* P, int _max_comp_time)
    : Problem(P->getInstanceFileName(), P->getG(), P->getMT(),
              P->getConfigStart(), P->getConfigGoal(), P->getNum(),
              P->getMaxTimestep(), _max_comp_time),
      instance_initialized(false)
{
}

MAPF_Instance::~MAPF_Instance()
{
  if (instance_initialized) {
    if (G != nullptr) delete G;
    if (MT != nullptr) delete MT;
  }
}

void MAPF_Instance::setRandomStartsGoals()
{
  // initialize
  config_s.clear();
  config_g.clear();

  // get grid size
  Grid* grid = reinterpret_cast<Grid*>(G);
  const int N = grid->getWidth() * grid->getHeight();

  // set starts
  std::vector<int> starts(N);
  std::iota(starts.begin(), starts.end(), 0);
  std::shuffle(starts.begin(), starts.end(), *MT);
  int i = 0;
  while (true) {
    while (G->getNode(starts[i]) == nullptr) {
      ++i;
      if (i >= N) halt("number of agents is too large.");
    }
    config_s.push_back(G->getNode(starts[i]));
    if ((int)config_s.size() == num_agents) break;
    ++i;
  }

  // set goals
  std::vector<int> goals(N);
  std::iota(goals.begin(), goals.end(), 0);
  std::shuffle(goals.begin(), goals.end(), *MT);
  int j = 0;
  while (true) {
    while (G->getNode(goals[j]) == nullptr) {
      ++j;
      if (j >= N) halt("set goal, number of agents is too large.");
    }
    // retry
    if (G->getNode(goals[j]) == config_s[config_g.size()]) {
      config_g.clear();
      std::shuffle(goals.begin(), goals.end(), *MT);
      j = 0;
      continue;
    }
    config_g.push_back(G->getNode(goals[j]));
    if ((int)config_g.size() == num_agents) break;
    ++j;
  }
}

/*
 * Note: it is hard to generate well-formed instances
 * with dense situations (e.g., ≥300 agents in arena)
 */
void MAPF_Instance::setWellFormedInstance()
{
  // initialize
  config_s.clear();
  config_g.clear();

  // get grid size
  const int N = G->getNodesSize();
  Nodes prohibited, starts_goals;

  while ((int)config_g.size() < getNum()) {
    while (true) {
      // determine start
      Node* s;
      do {
        s = G->getNode(getRandomInt(0, N - 1, MT));
      } while (s == nullptr || inArray(s, prohibited));

      // determine goal
      Node* g;
      do {
        g = G->getNode(getRandomInt(0, N - 1, MT));
      } while (g == nullptr || g == s || inArray(g, prohibited));

      // ensure well formed property
      auto path = G->getPath(s, g, starts_goals);
      if (!path.empty()) {
        config_s.push_back(s);
        config_g.push_back(g);
        starts_goals.push_back(s);
        starts_goals.push_back(g);
        for (auto v : path) {
          if (!inArray(v, prohibited)) prohibited.push_back(v);
        }
        break;
      }
    }
  }
}

void MAPF_Instance::makeScenFile(const std::string& output_file)
{
  Grid* grid = reinterpret_cast<Grid*>(G);
  std::ofstream log;
  log.open(output_file, std::ios::out);
  log << "map_file=" << grid->getMapFileName() << "\n";
  log << "agents=" << num_agents << "\n";
  log << "seed=0\n";
  log << "random_problem=0\n";
  log << "max_timestep=" << max_timestep << "\n";
  log << "max_comp_time=" << max_comp_time << "\n";
  for (int i = 0; i < num_agents; ++i) {
    log << config_s[i]->pos.x << "," << config_s[i]->pos.y << ","
        << config_g[i]->pos.x << "," << config_g[i]->pos.y << "\n";
  }
  log.close();
}

void MAPD_Instance::read_map_file()
{
  using namespace std;

  G = new Grid(map_file);
  ifstream file(map_file);
  if (file.is_open()) {
    // We initialize the value
    string line, value;

    // We read the number of row
    file >> value;
    int rows = stoi(value);
    // instance->set_nb_row(stoi(value));

    // // We read the number of column
    file >> value;
    int cols = stoi(value);
    // instance->set_nb_column(stoi(value));

    // // We read the number of endpoints
    file >> value;
    // int endpoints = stoi(value);
    // instance->set_nb_endpoint(stoi(value));

    // // We create the list of agents
    file >> value;
    num_agents = stoi(value);

    // // We read the not used value
    file >> value;
    // int max_horizon = stoi(value);
    // instance->set_max_horizon(max_horizon);

    // // We read the end of the line
    getline(file, line);

    // // We initialize the values
    int nb_found_agents = 0, current_node_id = -1;

    // // For each row
    for (int row = 0; row < rows; ++row) {
      // We get the corresponding line
      getline(file, line);

      // For each column
      for (int column = 0; column < cols; ++column) {
        // We increment the id of the current node
        ++current_node_id;

        // We get the next node value
        value = line.at(column);

        if (G->existNode(column, row)) {
          // Add endpoint node to list
          auto node = G->getNode(column, row);
          LOCATIONS.push_back(node);
        }

        if (value == "r") {
          Node* agent = G->getNode(column, row);
          config_s.push_back(agent);

          //             // We update the values for the instance
          //             instance->get_list_map_nodes().push_back(true);
          //             instance->get_list_endpoints().push_back(true);

          //             // We create a new agent for the problem
          //             instance->get_list_agents().push_back(new
          //             Agent(nb_found_agents,
          //                                                             row*instance->get_nb_column()
          //                                                             +
          //                                                             column,
          //                                                             max_horizon));

          //             instance->get_list_not_possible_endpoints().push_back(row*instance->get_nb_column()
          //             + column);
          //             instance->get_deadline_per_not_feasible_endpoint().push_back(0);

          //             // We increment the number of found agents
          //             ++ nb_found_agents;
          //         }
          //         else {

          //             // We update the values for the instance
          //             instance->get_list_map_nodes().push_back(false);
          //             instance->get_list_endpoints().push_back(false);
        }
      }
    }
  }

  // // We check that the number of created agents correspond
  // if (nb_agents != instance->get_nb_agent()){
  //     cout << "Problem, the number of agents does not correspond" << endl;
  //     getchar();
  // }

  // // We check that the number of created endpoints correspond
  // if (nb_found_endpoint != instance->get_nb_endpoint()){
  //     cout << "Problem, the number of endpoints does not correspond" <<
  //     endl; getchar();
  // }

  // // We check that the number of node correspond
  // if (instance->get_nb_row()*instance->get_nb_column() !=
  // instance->get_list_map_nodes().size()){
  //     cout << "Problem, the number of nodes does not correspond" << endl;
  //     getchar();
  // }

  // We close the file
  file.close();
}

// Simon #6
/**
 * @brief Reads tasks from a task file
 *
 */
void MAPD_Instance::read_task_file(bool is_batched)
{
  std::ifstream file(task_file);
  if (!file) halt("Couldn't open task file " + task_file);
  if (file.is_open()) {
    std::string line, value;
    getline(file, line);

    // We read the number of task
    task_num = stoi(line);

    int last_release = 0;
    int current_batch = -1;
    // For each task
    for (int task = 0; task < task_num; ++task) {
      // We get the release date
      file >> value;
      int release_date = stoi(value);

      if (release_date > last_release) last_release = release_date;

      // We get the pickup node
      file >> value;
      int p = stoi(value);

      // We get the delivery node
      file >> value;
      int d = stoi(value);

      // We don't use the last two values
      file >> value;
      file >> value;

      int batch_id = -1;
      // Read the batch id
      if (is_batched) {
        file >> value;
        int batch_id = stoi(value);

        // Create batch if it doesn't exist
        if (batch_id > current_batch) {
          current_batch++;
          Batch* batch = new Batch(batch_id);
          batches.push_back(batch);
        }
      }

      Node* pickup = get_endpoint_node_from_int(p);
      Node* delivery = get_endpoint_node_from_int(d);

      Task* task_obj = new Task(pickup, delivery, release_date, batch_id);
      if (is_batched) batches[current_batch]->add_task(task_obj);
      // We create a new task in the instance's list
      TASKS.push_back(task_obj);
    }
    file.close();

    // Index tasks by release time
    TASKS_SCHEDULED.resize(last_release + 1);
    for (auto task : TASKS) {
      TASKS_SCHEDULED[task->timestep_appear].push_back(task);
    }
  }
}

// -------------------------------------------
// MAPD
MAPD_Instance::MAPD_Instance(const std::string& _task_file,
                             const std::string& _map_file,
                             const bool _is_batched)
    : Problem(""),
      current_timestep(-1),
      specify_pickup_deliv_locs(true),
      task_file(_task_file),
      map_file(_map_file),
      is_batched(_is_batched)
{
  // Read map file
  read_map_file();
  std::string line;

  // set default value not identified params
  if (MT == nullptr) MT = new std::mt19937(DEFAULT_SEED);
  max_timestep = DEFAULT_MAX_TIMESTEP;
  max_comp_time = DEFAULT_MAX_COMP_TIME;

  // if (specify_pickup_deliv_locs) setupSpetialNodes();
  if (LOCS_PICKUP.empty()) {
    LOCS_PICKUP = G->getV();
    LOCS_DELIVERY = G->getV();
  }
  read_task_file(is_batched);
  // check starts
  if (num_agents <= 0) halt("invalid number of agents");
  if (num_agents > (int)config_s.size()) {
    if (!config_s.empty()) {
      warn("given starts are not sufficient\nrandomly create instances");
      config_s.clear();
    }
  }

  // trimming
  config_s.resize(num_agents);

  // initialize
  update();
}

MAPD_Instance::~MAPD_Instance()
{
  // for (auto task : TASKS_OPEN) delete task;
  // for (auto task : TASKS_CLOSED) delete task;
  for (auto task : TASKS) delete task;
}

// Simon #6
/**
 * @brief Gets a node from a node number (sequential num instead of x, y)
 *
 * @param num Node number
 * @return Node
 */
Node* MAPD_Instance::get_endpoint_node_from_int(int num)
{
  if (num > LOCATIONS.size()) {
    halt("Node number " + std::to_string(num) +
         " is greater than endpoints size");
  }
  return LOCATIONS[num];
}

void MAPD_Instance::setupSpetialNodes()
{
  Grid* grid = reinterpret_cast<Grid*>(G);

  // read instance file
#ifdef _MAPDIR_
  std::ifstream file(_MAPDIR_ + grid->getMapFileName() + ".pd");
#else
  std::ifstream file(grid->getMapFileName() + ".pd");
#endif
  if (!file) return;

  std::string line;
  const int width = grid->getWidth();

  int y = 0;
  while (getline(file, line)) {
    // for CRLF coding
    if (*(line.end() - 1) == 0x0d) line.pop_back();

    if ((int)line.size() != width) halt("pd format is invalid");

    for (int x = 0; x < width; ++x) {
      if (!G->existNode(x, y)) continue;

      auto v = G->getNode(x, y);
      LOCATIONS.push_back(v);
    }
    ++y;
  }
}

void MAPD_Instance::update()
{
  // check finished tasks
  auto itr = TASKS_OPEN.begin();
  while (itr != TASKS_OPEN.end()) {
    auto task = *itr;

    // not at delivery location
    if (task->loc_current != task->loc_delivery) {
      ++itr;
      continue;
    }

    task->timestep_finished = current_timestep + 1;
    TASKS_CLOSED.push_back(task);

    // remove from OPEN list
    itr = TASKS_OPEN.erase(itr);
  }

  // Release the tasks per their release timestep
  if (current_timestep < TASKS_SCHEDULED.size()) {
    for (auto task : TASKS_SCHEDULED[current_timestep]) {
      task->timestep_appear = current_timestep + 1;  // It will be handled in
      // the next loop, hence +1
      TASKS_OPEN.push_back(task);
    }
  }

  // update timestep
  ++current_timestep;
};
