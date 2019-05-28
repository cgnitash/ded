#pragma once

#include "../../components.h"

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

class pass_through {

  life::environment_spec env_{"null_environment"};

public:
  pass_through() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es{"pass_through"};
	/*
    c["pre-tags"] = nullptr;
    c["post-tags"]  = nullptr;
    c["input-tags"]  = nullptr;
    c["output-tags"] = nullptr;
    c["parameters"]["env"] = {
      env_[0], {}, {}, {}
    };   // and propogate population requirements
	*/
	es.bind_environment("env",env_);
    return es;
  }

  void configure(life::environment_spec es) { 
	  es.configure_environment("env",env_);
  }

  life::population evaluate(life::population pop)
  {
    auto env = life::make_environment(env_);
    return env.evaluate(pop);
  }
};
