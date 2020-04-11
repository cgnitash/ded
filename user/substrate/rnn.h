
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

  ded::encoding genome_;
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  std::vector<double> buffer_;

public:
  rnn() { configure(publishConfiguration()); }

  ded::configuration publishConfiguration()
  {
    ded::configuration c;
    c["parameters"]["inputs"]  = input_;
    c["parameters"]["outputs"] = output_;
    c["parameters"]["recurr"] = recurr_;

    c["input-tags"]["in-sense"]   = in_sense_;
    c["output-tags"]["out-sense"] = out_sense_;

    return c;
  }

  void configure(ded::configuration con)
  {
    input_  = con["parameters"]["inputs"];
    output_ = con["parameters"]["outputs"];
    recurr_ = con["parameters"]["recurr"];

    in_sense_  = con["input-tags"]["in-sense"];
    out_sense_ = con["output-tags"]["out-sense"];

    genome_.generate((input_ + recurr_) * (recurr_ + output_));
    buffer_ = std::vector<double>(input_ + recurr_ + output_, 0.);
  }

  void           reset();
  void           mutate();
  void           input(std::string, ded::concepts::Signal);
  ded::concepts::Signal   output(std::string);
  void           tick();
  ded::encoding getEncoding() const { return genome_; }
  void           setEncoding(ded::encoding e)
  {
    genome_ = e;
  }
  ded::encoding parseEncoding(std::string);
};
