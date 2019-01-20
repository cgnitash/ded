#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class markov2in1out {

  std::string in_sense_ = "in-sense,A<double,inputs>";
  std::string out_sense_ = "out-sense,A<double,outputs>";

  size_t input_ = 4;
  size_t output_ = 2;
  size_t hidden_ = 4;

  life::encoding genome_;
  std::regex encoding_parser_{R"(([^:]+):)"};

  std::vector<double> buffer_;

  struct gate {
    size_t in_1_, in_2_, out_;
    std::array<long, 4> logic_;
  };

  std::vector<gate> gates_;

  bool gates_are_computed_ = false;
  void compute_gates_();
  void seed_gates_(size_t = 1);

public:
  markov2in1out() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["parameters"]["inputs"] = input_;
    c["parameters"]["outputs"] = output_;
    c["parameters"]["hiddens"] = hidden_;

	c["input-tags"]["in-sense"] = in_sense_;
	c["output-tags"]["out-sense"] = out_sense_;

    return c;
  }

  void configure(life::configuration con) {
    input_ = con["parameters"]["inputs"];
    output_ = con["parameters"]["outputs"];
    hidden_ = con["parameters"]["hiddens"];

    in_sense_ = con["input-tags"]["in-sense"];
    out_sense_ = con["output-tags"]["out-sense"]; 

    genome_.generate(100);
    buffer_ = std::vector<double>(input_ + output_ + hidden_, 0.);
    seed_gates_(4);
  }

  void mutate();
  void input(std::string, life::signal);
  life::signal output(std::string );
  void tick();
  life::encoding get_encoding() const { return genome_; }
  void set_encoding(life::encoding e) {
    genome_ = e;
    compute_gates_();
  }
  life::encoding parse_encoding(std::string);
};
