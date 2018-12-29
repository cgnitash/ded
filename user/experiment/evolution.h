
#pragma once

#include "../../components.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

class evolution {

  long generations_ = 50;
  std::string pop_name_  = "null_population";
  std::string sel_name_ = "null_environment";
  std::string world_name_ = "null_environment";
  life::configuration pop_config_;
  life::configuration sel_config_;
  life::configuration world_config_;
  long Seed_ = 0;

public:
  evolution() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["parameters"]["population"] = {pop_name_,{}};
    ec["parameters"]["selector"] = {sel_name_,{}};
    ec["parameters"]["fitness"] = {world_name_,{}};
    ec["parameters"]["generations"] = generations_;
    ec["parameters"]["SEED"] = Seed_;
    return ec;
  }

  void configure(life::configuration con) {
    generations_ = con["parameters"]["generations"];
    Seed_ = con["parameters"]["SEED"];
	pop_name_ = std::string(con["parameters"]["population"][0]);
	pop_config_ = con["parameters"]["population"][1];
	sel_name_ = std::string(con["parameters"]["selector"][0]);
	sel_config_ = con["parameters"]["selector"][1];
	world_name_= std::string(con["parameters"]["fitness"][0]);
	world_config_ = con["parameters"]["fitness"][1];

	std::srand(Seed_);
  }

  void run();
};

