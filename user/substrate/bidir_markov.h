#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class bidir_markov {

  std::string in_straight_ = "straight,A<double,inputs>";
  std::string in_reverse_  = "reverse,A<double,inputs>";
  std::string out_sense_   = "out-sense,A<double,outputs>";

  ded::encoding genome_;
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  size_t input_  = 10;
  size_t output_ = 10;
  size_t hidden_ = 10;

  ded::configuration genome_config_;

  std::vector<double> buffer_;

  std::vector<long> codon_{ 7, 14 };
  long              gene_length_ = 28;

  struct gate
  {
    std::vector<long> logic_, ins_, outs_;
  };

  std::vector<gate> gates_;

  bool gates_are_computed_ = false;
  void compute_gates_();
  void seed_gates_(size_t = 1);

public:
  bidir_markov() { configure(publishConfiguration()); }
  ded::configuration publishConfiguration()
  {
    ded::configuration con;
    con["parameters"]["inputs"]        = input_;
    con["parameters"]["outputs"]       = output_;
    con["parameters"]["hiddens"]       = hidden_;
    con["parameters"]["genome-params"] = genome_config_;

    con["input-tags"]["straight"]   = in_straight_;
    con["input-tags"]["reverse"]    = in_reverse_;
    con["output-tags"]["out-sense"] = out_sense_;

    return con;
  }
  void configure(ded::configuration con)
  {
    input_         = con["parameters"]["inputs"];
    output_        = con["parameters"]["outputs"];
    hidden_        = con["parameters"]["hiddens"];
    genome_config_ = con["parameters"]["genome-params"];

    in_straight_ = con["input-tags"]["straight"];
    in_reverse_  = con["input-tags"]["reverse"];
    out_sense_   = con["output-tags"]["out-sense"];

    genome_.configure(genome_config_);
    genome_.generate(500);
    buffer_ = std::vector(input_ + output_ + hidden_, 0.);
    seed_gates_(4);
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
    compute_gates_();
  }
  ded::encoding parseEncoding(std::string);
};
