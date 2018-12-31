#pragma once 

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
#include <set>
#include <initializer_list>

class null_environment {

public:
  null_environment() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration con;
    con["parameters"] = nullptr;
    con["pre-tags"] = nullptr;
    con["post-tags"] = nullptr;
    return con;
  }

  void configure(life::configuration ) {
  }

  life::population evaluate(life::population);
};

