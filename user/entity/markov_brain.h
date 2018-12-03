# pragma once

#include"../../core/configuration.h"
#include"../../core/signal.h"
#include"../../core/encoding.h"

#include <vector>
#include <algorithm>

class markov_brain {

  life::encoding genome_;
  size_t input_ = 10;
  size_t output_ = 10;
  size_t hidden_ = 10;
  
  life::configuration genome_config_;

  std::vector<double> buffer_;

  struct gate {
    std::vector<long> logic_, ins_, outs_;
  };

  std::vector<gate> gates_;

  void compute_gates_();
  void seed_gates_(size_t = 1);

public:
  markov_brain() { configure(publish_configuration()); }
  life::configuration publish_configuration() {
    life::configuration con;
    con["inputs"] = input_;
    con["outputs"] = output_;
    con["hiddens"] = hidden_;
    con["genome-params"] = genome_config_;
    return con;
  }
  void configure(life::configuration con) {
    input_ = con["inputs"];
    output_ = con["outputs"];
    hidden_ = con["hiddens"];
    genome_config_ = con["genome-params"];
    genome_.configure(genome_config_);
    genome_.generate(500);
    buffer_ = std::vector(input_ + output_ + hidden_, 0.);
    seed_gates_(4);
	compute_gates_();
  }

  void seed_gates(size_t);
  void mutate();
  void input(life::signal);
  life::signal output();
  void tick();
  life::encoding get_encoding() const { return genome_; }
};
