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

class null_environment {

public:
  null_environment() { configure(publish_configuration()); }

  life::configuration publish_configuration()
  {
    life::configuration con;
    con["parameters"]  = nullptr;
    con["pre-tags"]    = nullptr;
    con["post-tags"]   = nullptr;
    con["input-tags"]  = nullptr;
    con["output-tags"] = nullptr;
    return con;
  }

  void configure(life::configuration) {}

  life::population evaluate(life::population);
};
