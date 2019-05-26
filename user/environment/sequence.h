
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

class sequence {

  // bool to_cout_ = false;

  //std::string score_tag_ = "score,double";

  life::environment_spec one_{ "null_environment" };
  life::environment_spec two_{ "null_environment" };

public:
  sequence() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es{"sequence"};

    //ec["parameters"]["to-cout"] = to_cout_;

    //ec["input-tags"]  = nullptr;
    //ec["output-tags"] = nullptr;

    //ec["pre-tags"]  = nullptr;
    //ec["post-tags"] = nullptr;

    // o:in:selector(P) must handle these tags
	es.bind_environment("one",one_);
    //ec["parameters"]["one"] = { one_[0], {}, { score_tag_ }, {} };

    // o:in:fitness(P) must provide these tags
	es.bind_environment("one",two_);
    //ec["parameters"]["two"] = { two_[0], {}, {}, { score_tag_ } };

	es.bind_tag_flow({"one", "post"}, {"two", "pre"});

    return es;
  }

  void configure(life::environment_spec es)
  {
    //es.configure_("to-cout",to_cout_);

    es.configure_environment("one",one_);
    es.configure_environment("two",two_);
  }

  life::population evaluate(life::population);
};
