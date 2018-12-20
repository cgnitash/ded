
#pragma once

#include "../../components.h"

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
#include <regex>

class cppn {

private:
  struct Node {
    size_t activation_function;
    std::map<size_t, double> in_node;
  };
  std::vector<Node> nodes_;
  size_t input_ = 1;
  size_t output_ = 1;
  size_t hidden_ = 0;

  life::encoding genome_;
  std::regex encoding_parser_{R"(([^:]):)"};

  std::vector<double> ins_ ;
  std::vector<double> outs_ ;

  double activate(size_t, double);
  void print();
  void compute_nodes_();
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
    genome_.generate(9 * (output_ + hidden_));
	compute_nodes_();
  }

  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
  life::encoding get_encoding() const { return genome_; }
  void set_encoding(life::encoding e) {
    genome_ = e;
    compute_nodes_();
  }
  life::encoding parse_encoding(std::string);
};

