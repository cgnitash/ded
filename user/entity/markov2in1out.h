#pragma once

#include "../../core/configuration.h"
#include "../../core/encoding.h"
#include "../../core/signal.h"

#include <algorithm>
#include <vector>

class markov2in1out {

  size_t input_ = 4;
  size_t output_ = 2;
  size_t hidden_ = 4;
  life::encoding genome_ = life::generate(100);

  std::vector<size_t> buffer_ =
      std::vector<size_t>(input_ + output_ + hidden_, 0u);

  struct gate {
    size_t in_1_, in_2_, out_;
    std::array<size_t, 4> logic_;
  };

  std::vector<gate> gates_;

  void compute_gates_();
  void seed_gates(size_t = 1);

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
    life::encoding genome_ = life::generate(100);
    buffer_ = std::vector<size_t>(input_ + output_ + hidden_, 0u);
    seed_gates(4);
  }

  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
};
