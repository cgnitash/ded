
#include"../../components.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <regex>


class evolution {

  long pop_size;
  long generations;
  //life::entity org ;
  life::selector optimiser ;
  life::environment world ;
  std::string org_name;
  life::configuration org_config;
public:
  evolution() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["entity"] = {"null_entity", {}, "the entities to be evolved"};
    ec["selector"] = {
        "null_selector", {}, "selection process used by evolution"};
    ec["environment"] = {
        "null_environment", {}, "selection process used by evolution"};
    ec["population_size"] = 100;
    ec["generations"] = 50;
    return ec;
  }

  void configure(life::configuration con) {

    pop_size = (con["population_size"]);
    generations = (con["generations"]);
    optimiser= life::make_selector(std::string(con["selector"][0]));
    optimiser.configure(con["selector"][1]);
    world = life::make_environment(std::string(con["environment"][0]));
    world.configure(con["environment"][1]);
	org_name = std::string(con["entity"][0]);
	org_config = con["entity"][1];
  }

  void run();
};

