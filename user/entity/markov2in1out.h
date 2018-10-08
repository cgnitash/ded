# pragma once

#include"../../core/configuration.h"
#include"../../core/signal.h"
#include"../../core/encoding.h"

#include <vector>
#include <algorithm>

class markov2in1out {

  life::encoding genome_;
  long input_;
  long output_;
  long hidden_;

  std::vector<long> buffer_;

  struct gate {
    long in_1_, in_2_, out_ ;
	std::array<long,4> logic_;
  };

  std::vector<gate> gates_;

	bool gates_valid_ = false;
	void compute_gates_();
        void seed_gates(long n = 1) {
          for (int i = 0; i < n; i++) {
            auto pos = rand() % (genome_.size() - 1);
            genome_[pos] = 'c';
            genome_[pos + 1] = 'd';
          }
        }

public:
  markov2in1out() { configure(publish_configuration()) ;}
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
    genome_ = std::string(100, 'a');
    buffer_ = std::vector<long>(input_ + output_ + hidden_, 0);
	seed_gates(20);
  }
  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
};
