# pragma once

#include"../../core/configuration.h"
#include"../../core/signal.h"
#include"../../core/encoding.h"

#include <vector>
#include <algorithm>

class markov_brain {

  life::encoding genome_;
  long input_;
  long output_;
  long hidden_;

  std::vector<long> buffer_;

  struct gate {
    std::vector<long> logic_, ins_, outs_;
  };

  std::vector<gate> gates_;

	bool gates_valid_ = false;
	void compute_gates_();

public:
  markov_brain() { configure(publish_configuration()) ;}
  life::configuration publish_configuration() {
    life::configuration c;
    c["inputs"] = 4;
    c["outputs"] = 2;
    c["hiddens"] = 4;
    return c;
  }
  void configure(life::configuration con) {
    input_ = con["inputs"];
    output_ = con["outputs"];
    hidden_ = con["hiddens"];
    genome_ = life::encoding(500, 0);
    buffer_ = std::vector<long>(input_ + output_ + hidden_, 0);
  }
  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
};
