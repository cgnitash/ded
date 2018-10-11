
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
  std::string org_name = "null_entity";
  std::string sel_name = "null_selector";
  std::string world_name = "null_environment";
  life::configuration org_config;
  life::configuration sel_config;
  life::configuration world_config;
public:
  evolution() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["entity"] = {org_name,{}};
    ec["selector"] = {sel_name,{}};
    ec["environment"] = {world_name,{}};
    ec["population_size"] = 100;
    ec["generations"] = 50;
    return ec;
  }

  void configure(life::configuration con) {
    pop_size = (con["population_size"]);
    generations = (con["generations"]);
	org_name = std::string(con["entity"][0]);
	org_config = con["entity"][1];
	sel_name = std::string(con["selector"][0]);
	sel_config = con["selector"][1];
	world_name= std::string(con["environment"][0]);
	world_config = con["environment"][1];
  }

  void run();
};

