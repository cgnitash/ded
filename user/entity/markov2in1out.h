#pragma once

#include "../../core/configuration.h"
#include "../../core/encoding.h"
#include "../../core/signal.h"

#include <algorithm>
#include <vector>

class markov2in1out {

  life::encoding genome_ = life::encoding(100l, 0);
  long input_ = 4;
  long output_ = 2;
  long hidden_ = 4;

  std::vector<double> buffer_ =
      std::vector<double>(input_ + output_ + hidden_, 0);

  struct gate {
    long in_1_, in_2_, out_;
    std::array<double, 4> logic_;
  };

  std::vector<gate> gates_;

  bool gates_valid_ = false;

  void compute_gates_();

  void seed_gates(long = 1);

  long bit(double d) { return d < 1.0 ? 0 : 1; }

public:
  markov2in1out() { configure(publish_configuration()); }
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
    buffer_ = std::vector<double>(input_ + output_ + hidden_, 0);
    seed_gates(6);
  }
  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
};
