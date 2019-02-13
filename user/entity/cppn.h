
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
#include <regex>
#include <utility>
#include <vector>

class cppn {

private:
  std::string in_sense_  = "in-sense,A<double,inputs>";
  std::string out_sense_ = "out-sense,A<double,outputs>";

  std::string init_enc_;

  struct Node
  {
    size_t                   activation_function;
    std::map<size_t, double> in_node;
  };
  std::vector<Node> nodes_;
  size_t            input_  = 1;
  size_t            output_ = 1;
  size_t            hidden_ = 0;

  life::encoding genome_;
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  std::vector<double> ins_;
  std::vector<double> outs_;

  double activate(size_t, double);
  void   print();
  bool   gates_are_computed_ = false;
  void   compute_nodes_();

public:
  cppn() { configure(publish_configuration()); }

  life::configuration publish_configuration()
  {
    life::configuration c;
    c["parameters"]["inputs"]  = input_;
    c["parameters"]["outputs"] = output_;
    c["parameters"]["hiddens"] = hidden_;
    c["parameters"]["enco"] = init_enc_;

    c["input-tags"]["in-sense"]   = in_sense_;
    c["output-tags"]["out-sense"] = out_sense_;

    return c;
  }

  void configure(life::configuration con)
  {
    input_  = con["parameters"]["inputs"];
    output_ = con["parameters"]["outputs"];
    hidden_ = con["parameters"]["hiddens"];

    init_enc_ = con["parameters"]["enco"];

    in_sense_  = con["input-tags"]["in-sense"];
    out_sense_ = con["output-tags"]["out-sense"];

    if (init_enc_.empty())
      genome_.generate(9 * (output_ + hidden_));
    else
      set_encoding(parse_encoding(init_enc_));
  }

  void           mutate();
  void           input(std::string, life::signal);
  life::signal   output(std::string);
  void           tick();
  life::encoding get_encoding() const { return genome_; }
  void           set_encoding(life::encoding e)
  {
    genome_ = e;
    compute_nodes_();
  }
  life::encoding parse_encoding(std::string);
};
