
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

class rnn_with_cppn {

private:
  std::string in_sense_  = "in-sense,A<double,inputs>";
  std::string out_sense_ = "out-sense,A<double,outputs>";

  std::string internal_cppn_input_tag{};
  std::string internal_cppn_output_tag{};

  size_t input_       = 1;
  size_t output_      = 1;
  size_t recurr_      = 0;

  size_t cppn_hidden_ = 0;
  size_t cppn_recurr_ = 0;
  double offset_ = 0.1;
  double source_x_{1};
  double source_y_{1};

  ded::encoding genome_;
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  std::vector<double> buffer_;
  ded::substrate internal_cppn_ = ded::makesubstrate({"cppn"});

  void reconstruct_weights_();
public:
  rnn_with_cppn() { configure(publishConfiguration()); }

  ded::configuration publishConfiguration()
  {
    ded::configuration c;
    c["parameters"]["inputs"]  = input_;
    c["parameters"]["outputs"] = output_;
    c["parameters"]["recurr"] = recurr_;

    c["parameters"]["offset"] = offset_;
    c["parameters"]["src-x"] = source_x_;
    c["parameters"]["src-y"] = source_y_;
    c["parameters"]["cppn-hidden"] = cppn_hidden_;
    c["parameters"]["cppn-recurr"] = cppn_recurr_;

    c["input-tags"]["in-sense"]   = in_sense_;
    c["output-tags"]["out-sense"] = out_sense_;

    return c;
  }

  void configure(ded::configuration con)
  {
    input_       = con["parameters"]["inputs"];
    output_      = con["parameters"]["outputs"];
    recurr_      = con["parameters"]["recurr"];

    offset_   = con["parameters"]["offset"];
    source_x_ = con["parameters"]["src-x"];
    source_y_ = con["parameters"]["src-y"];

    cppn_hidden_ = con["parameters"]["cppn-hidden"];
    cppn_recurr_ = con["parameters"]["cppn-recurr"];

    in_sense_  = con["input-tags"]["in-sense"];
    out_sense_ = con["output-tags"]["out-sense"];

    ded::configuration c = internal_cppn_.publishConfiguration();
    // yucky
    internal_cppn_input_tag    = c["input-tags"]["in-sense"];
    internal_cppn_output_tag   = c["output-tags"]["out-sense"];

    c["parameters"]["inputs"]  = 2;
    c["parameters"]["outputs"] = 1;
    c["parameters"]["hiddens"] = cppn_hidden_;
    c["parameters"]["recurr"] = cppn_recurr_;
    internal_cppn_.configure(c);
    genome_.generate((input_ + recurr_) * (recurr_ + output_));
    reconstruct_weights_();
    buffer_ = std::vector<double>(input_ + recurr_ + output_, 0.);
  }

  void           mutate();
  void           reset();
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
