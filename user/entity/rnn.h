
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

class rnn {

private:
  std::string in_sense_  = "in-sense,A<double,inputs>";
  std::string out_sense_ = "out-sense,A<double,outputs>";

  size_t input_  = 1;
  size_t output_ = 1;
  size_t recurr_ = 0;

  life::encoding genome_;
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  std::vector<double> buffer_;

public:
  rnn() { configure(publish_configuration()); }

  life::configuration publish_configuration()
  {
    life::configuration c;
    c["parameters"]["inputs"]  = input_;
    c["parameters"]["outputs"] = output_;
    c["parameters"]["recurr"] = recurr_;

    c["input-tags"]["in-sense"]   = in_sense_;
    c["output-tags"]["out-sense"] = out_sense_;

    return c;
  }

  void configure(life::configuration con)
  {
    input_  = con["parameters"]["inputs"];
    output_ = con["parameters"]["outputs"];
    recurr_ = con["parameters"]["recurr"];

    in_sense_  = con["input-tags"]["in-sense"];
    out_sense_ = con["output-tags"]["out-sense"];

    genome_.generate((input_ + recurr_) * (recurr_ + output_));
    buffer_ = std::vector<double>(input_ + recurr_ + output_, 0.);
  }

  void           mutate();
  void           input(std::string, life::signal);
  life::signal   output(std::string);
  void           tick();
  life::encoding get_encoding() const { return genome_; }
  void           set_encoding(life::encoding e)
  {
    genome_ = e;
  }
  life::encoding parse_encoding(std::string);
};
