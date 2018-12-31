
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

class evolution_loop {

  long generations_ = 50;

  std::string score_tag_  = "score";
  
  std::string sel_name_ = "null_environment";
  std::string world_name_ = "null_environment";
  life::configuration sel_config_;
  life::configuration world_config_;
  long Seed_ = 0;

public:
  evolution_loop() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["parameters"]["generations"] = generations_;
    ec["parameters"]["SEED"] = Seed_;

    // o:in:P has no tags
    ec["pre-tags"] = nullptr;

    // o:in:P' has no tags
    ec["post-tags"] = nullptr;

	// o:in:selector(P) must handle these tags
    ec["parameters"]["selector"] = {sel_name_, {}, {score_tag_}, {}};

    // o:in:fitness(P) must provide these tags
    ec["parameters"]["fitness"] = {world_name_, {}, {}, {score_tag_}};

    return ec;
  }

  void configure(life::configuration con) {
    generations_ = con["parameters"]["generations"];
    Seed_ = con["parameters"]["SEED"];
	sel_name_ = std::string(con["parameters"]["selector"][0]);
	sel_config_ = con["parameters"]["selector"][1];
	world_name_= std::string(con["parameters"]["fitness"][0]);
	world_config_ = con["parameters"]["fitness"][1];

	std::srand(Seed_);
  }

  life::population evaluate(life::population);
};

