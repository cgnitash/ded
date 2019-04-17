#pragma once

#include "../../components.h"

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class exit_room {

  std::string score_tag_ = "score,double";

  std::string org_input_vision_tag_  = "vision,A<double,5>";
  std::string org_output_action_tag_ = "action,A<double,1>";

  size_t updates_    = 100;
  size_t num_copies_ = 2;
  size_t num_trials_ = 2;

  struct clone {
	  life::entity e;
	  size_t id, position;
  };

  double eval(life::entity &);

public:
  exit_room() { configure(publish_configuration()); }

  life::configuration publish_configuration()
  {
    life::configuration ec;
    ec["parameters"]["updates"]    = updates_;
    ec["parameters"]["num-copies"] = num_copies_;
    ec["parameters"]["num-trials"] = num_trials_;

    ec["pre-tags"] = nullptr;

    ec["post-tags"]["score"] = score_tag_;

    ec["input-tags"]["vision"]  = org_input_vision_tag_;
    ec["output-tags"]["action"] = org_output_action_tag_;
    return ec;
  }

  void configure(life::configuration con)
  {

    updates_ = con["parameters"]["updates"];
    num_copies_= con["parameters"]["num-copies"] ;
    num_trials_= con["parameters"]["num-trials"] ;

    score_tag_ = con["post-tags"]["score"];

    org_input_vision_tag_  = con["input-tags"]["vision"];
    org_output_action_tag_ = con["output-tags"]["action"];
  }

  life::population evaluate(life::population);
};
