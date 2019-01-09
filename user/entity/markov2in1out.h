#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class markov2in1out {

  size_t input_ = 4;
  size_t output_ = 2;
  size_t hidden_ = 4;
  life::encoding genome_;

  std::vector<double> buffer_;

  struct gate {
    size_t in_1_, in_2_, out_;
    std::array<long, 4> logic_;
  };

  std::vector<gate> gates_;

  void compute_gates_();
  void seed_gates_(size_t = 1);

public:
  markov2in1out() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["parameters"]["inputs"] = input_;
    c["parameters"]["outputs"] = output_;
    c["parameters"]["hiddens"] = hidden_;
    return c;
  }

  void configure(life::configuration con) {
    input_ = con["parameters"]["inputs"];
    output_ = con["parameters"]["outputs"];
    hidden_ = con["parameters"]["hiddens"];
    genome_.generate(100);
    buffer_ = std::vector<double>(input_ + output_ + hidden_, 0.);
    seed_gates_(4);
	compute_gates_();
  }

  void mutate();
  void input(std::string, life::signal);
  life::signal output(std::string );
  void tick();
};
