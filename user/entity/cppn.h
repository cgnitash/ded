
#pragma once

#include "../../core/configuration.h"
#include "../../core/encoding.h"
#include "../../core/signal.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <utility>
#include <vector>

class cppn {

private:
  struct Node {
    size_t activation_function;
    std::map<size_t, double> in_node;
  };
  std::vector<Node> nodes;
  size_t input_ = 1;
  size_t output_ = 1;
  size_t hidden_ = 0;
  life::encoding genome_;

  std::vector<double> ins_ ;
  std::vector<double> outs_ ;

  double activate(size_t, double);
  void print();
public:
  cppn() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["inputs"] = input_;
    c["outputs"] = output_;
    c["hiddens"] = hidden_;
    return c;
  }

  void configure(life::configuration con) {
    input_ = con["inputs"];
    output_ = con["outputs"];
    hidden_ = con["hiddens"];
    genome_ = life::generate(9 * (output_ + hidden_));
  }

  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
};

