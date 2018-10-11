
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

  long pop_size_;
  long generations_;
  //life::entity org ;
  std::string org_name_ = "null_entity";
  std::string sel_name_ = "null_selector";
  std::string world_name_ = "null_environment";
  life::configuration org_config_;
  life::configuration sel_config_;
  life::configuration world_config_;
public:
  evolution() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["entity"] = {org_name_,{}};
    ec["selector"] = {sel_name_,{}};
    ec["environment"] = {world_name_,{}};
    ec["population_size"] = 100;
    ec["generations"] = 50;
    return ec;
  }

  void configure(life::configuration con) {
    pop_size_ = (con["population_size"]);
    generations_ = (con["generations"]);
	org_name_ = std::string(con["entity"][0]);
	org_config_ = con["entity"][1];
	sel_name_ = std::string(con["selector"][0]);
	sel_config_ = con["selector"][1];
	world_name_= std::string(con["environment"][0]);
	world_config_ = con["environment"][1];
  }

  void run();
};

