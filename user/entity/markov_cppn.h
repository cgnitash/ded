# pragma once

#include"../../core/configuration.h"
#include"../../core/signal.h"
#include"../../core/encoding.h"
#include"../../core/entity.h"

#include <vector>
#include <algorithm>

class markov_cppn {

  life::encoding genome_;
  std::regex encoding_parser_{R"(([^:]):)"};

  size_t input_ = 10;
  size_t output_ = 10;
  size_t hidden_ = 10;
  
  bool mutate_wires_{false};
  bool mutate_weights_{false};
  bool mutate_acfns_{false};
  bool mutate_cppn_{false};
  
  life::configuration genome_config_;

  life::entity my_cppn_ = life::make_entity("cppn");

  std::vector<double> buffer_;

  std::vector<long> codon_{3, 35};
  long gene_length_ = 16;

  struct gate {
    std::vector<long> logic_, ins_, outs_;
  };

  std::vector<gate> gates_;

  void compute_gates_();
  void seed_gates_(size_t = 1);

public:
  markov_cppn() { configure(publish_configuration()); }
  life::configuration publish_configuration() {
    life::configuration con;
    con["inputs"] = input_;
    con["outputs"] = output_;
    con["hiddens"] = hidden_;
    con["mutate-wires"] = mutate_wires_;
    con["mutate-weights"] = mutate_weights_;
    con["mutate-acfns"] = mutate_acfns_;
    con["mutate-cppn"] = mutate_cppn_;
    con["genome-params"] = genome_config_;
    return con;
  }
  void configure(life::configuration con) {
    input_ = con["inputs"];
    output_ = con["outputs"];
    hidden_ = con["hiddens"];
    mutate_wires_ = con["mutate-wires"];
    mutate_weights_ = con["mutate-weights"];
    mutate_acfns_ = con["mutate-acfns"];
    mutate_cppn_ = con["mutate-cppn"];
    genome_config_ = con["genome-params"];
    genome_.configure(genome_config_);
    genome_.generate(200);
    my_cppn_.configure({{"inputs", 2}, {"outputs", 1}, {"hiddens", 2}});
    my_cppn_.set_encoding(
        my_cppn_.parse_encoding("1:0:97:2:101:98:21:88:1:"
                                "2:12:76:20:10:100:62:62:4:"
                                "0:66:59:28:92:93:38:123:101:"));
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
  void set_encoding(life::encoding e) {
    genome_ = e;
    compute_gates_();
  }
  life::encoding parse_encoding(std::string);
};
