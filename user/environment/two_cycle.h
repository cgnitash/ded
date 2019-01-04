
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

class two_cycle {

  long count_ = 1;
  long cout_freq_ = 0;
  long snap_freq_ = 0;
  long pop_av_freq_ = 0;

  std::string score_tag_  = "score";
  
  std::string sel_name_ = "null_environment";
  std::string fitn_name_ = "null_environment";
  life::configuration sel_config_;
  life::configuration fitn_config_;

  // not here??
  long Seed_ = 0;

public:
  two_cycle() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["parameters"]["count"] = count_;
    ec["parameters"]["snapshot-frequency"] = snap_freq_;
    ec["parameters"]["cout-frequency"] = cout_freq_;
    ec["parameters"]["pop-avmax-frequency"] = pop_av_freq_;
    //ec["parameters"]["SEED"] = Seed_;

    // o:in:P has no tags
    ec["pre-tags"] = nullptr;

    // o:in:P' has no tags
    ec["post-tags"] = nullptr;

	// o:in:selector(P) must handle these tags
    ec["parameters"]["selector"] = {sel_name_, {}, {score_tag_}, {}};

    // o:in:fitness(P) must provide these tags
    ec["parameters"]["fitness"] = {fitn_name_, {}, {}, {score_tag_}};

    return ec;
  }

  void configure(life::configuration con) {
    count_ = con["parameters"]["count"];
    snap_freq_ = con["parameters"]["snapshot-frequency"];
    cout_freq_ = con["parameters"]["cout-frequency"];
    pop_av_freq_ = con["parameters"]["pop-avmax-frequency"];
    //Seed_ = con["parameters"]["SEED"];
	
	sel_name_ = std::string(con["parameters"]["selector"][0]);
	sel_config_ = con["parameters"]["selector"][1];
	fitn_name_ = std::string(con["parameters"]["fitness"][0]);
	fitn_config_ = con["parameters"]["fitness"][1];

	//std::srand(Seed_);
  }

  life::population evaluate(life::population);
};

