
#pragma once

#include "../../core/configuration.h"
#include "../../core/encoding.h"
#include "../../core/signal.h"

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
  const double PI = std::atan(1) * 4;
  struct Node {
    long activation_function;
    std::map<int, double> in_node;
  };
  std::vector<Node> nodes;
  long input_ = 1;
  long output_ = 1;
  long hidden_ = 0;
  life::encoding genome_ = life::generate(9 * (output_ + hidden_));
  std::vector<double> ins_ = std::vector(input_, 0.0);
  std::vector<double> outs_ = std::vector(output_, 0.0);

  double activate(long, double);
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
  }
  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
};

