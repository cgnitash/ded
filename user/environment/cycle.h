
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

class cycle {

  long count_ = 1;
  long snap_freq_ = 0;

  std::string score_tag_  = "score";
  
  life::configuration world_{"null_environment", {}};

  // not here??
  long Seed_ = 0;

public:
  cycle() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["parameters"]["count"] = count_;
    ec["parameters"]["snapshot-frequency"] = snap_freq_;
    //ec["parameters"]["SEED"] = Seed_;

    // o:in:P has no tags
    ec["pre-tags"] = nullptr;

    // o:in:P' has no tags
    ec["post-tags"] = nullptr;

    // o:in:fitness(P) must provide these tags
    ec["parameters"]["world"] = {world_[0], {}, {}, {score_tag_}};

    return ec;
  }

  void configure(life::configuration con) {
    count_ = con["parameters"]["count"];
    snap_freq_ = con["parameters"]["snapshot-frequency"];
    //Seed_ = con["parameters"]["SEED"];
	
	world_= con["parameters"]["world"];

	//std::srand(Seed_);
  }

  life::population evaluate(life::population);
};

