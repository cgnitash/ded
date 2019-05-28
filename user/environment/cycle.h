
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

  long count_     = 1;
  //long snap_freq_ = 0;

  std::string score_tag_ = "score,double";

  life::environment_spec world_{"null_environment"};

  // not here??
  // long Seed_ = 0;

public:
  cycle() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es{"cycle"};
    es.bind_parameter("count",count_);
    //ec["parameters"]["snapshot-frequency"] = snap_freq_;
    // ec["parameters"]["SEED"] = Seed_;

    //ec["input-tags"]  = nullptr;
    //ec["output-tags"] = nullptr;

    //ec["pre-tags"]  = nullptr;
    //ec["post-tags"] = nullptr;

    // o:in:fitness(P) must provide these tags
	es.bind_environment("world",world_);
	es.bind_tag_flow({"world","pre"},{"world","post"});
    //ec["parameters"]["world"] = { world_[0], {}, {}, { score_tag_ } };

    return es;
  }

  void configure(life::environment_spec es)
  {
    es.configure_parameter("count",count_);
    //snap_freq_ = con["parameters"]["snapshot-frequency"];
    // Seed_ = con["parameters"]["SEED"];

    es.configure_environment("world",world_);

    // std::srand(Seed_);
  }

  life::population evaluate(life::population);
};
