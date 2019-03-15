#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class markov_with_cppn {

  std::string in_sense_  = "in-sense,A<double,inputs>";
  std::string out_sense_ = "out-sense,A<double,outputs>";

  life::encoding genome_;
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  std::string internal_cppn_input_tag{};
  std::string internal_cppn_output_tag{};

  size_t input_  = 10;
  size_t output_ = 10;
  size_t hidden_ = 10;

  size_t cppn_hidden_ = 0;
  size_t cppn_recurr_ = 0;
  double offset_ = 0.1;
  double source_x_{1};
  double source_y_{1};

  bool mutate_wires_{ false };
  bool mutate_weights_{ false };
  bool mutate_acfns_{ false };
  bool mutate_cppn_{ false };

  life::configuration genome_config_;

  life::entity my_cppn_ = life::make_entity({ "cppn" });

  std::vector<double> buffer_;

  std::vector<long> codon_{ 3, 35 };
  long              gene_length_ = 16;

  struct gate
  {
    std::vector<long> logic_, ins_, outs_;
  };

  std::vector<gate> gates_;

  bool gates_are_computed_ = false;
  void compute_gates_();
  void seed_gates_(size_t = 1);

public:
  markov_with_cppn() { configure(publish_configuration()); }
  life::configuration publish_configuration()
  {
    life::configuration con;
    con["parameters"]["inputs"]         = input_;
    con["parameters"]["outputs"]        = output_;
    con["parameters"]["hiddens"]        = hidden_;
    con["parameters"]["mutate-wires"]   = mutate_wires_;
    con["parameters"]["mutate-weights"] = mutate_weights_;
    con["parameters"]["mutate-acfns"]   = mutate_acfns_;
    con["parameters"]["mutate-cppn"]    = mutate_cppn_;
    con["parameters"]["genome-params"]  = genome_config_;

    con["parameters"]["offset"] = offset_;
    con["parameters"]["src-x"] = source_x_;
    con["parameters"]["src-y"] = source_y_;
    con["parameters"]["cppn-hidden"] = cppn_hidden_;
    con["parameters"]["cppn-recurr"] = cppn_recurr_;

    con["input-tags"]["in-sense"]   = in_sense_;
    con["output-tags"]["out-sense"] = out_sense_;

    return con;
  }
  void configure(life::configuration con)
  {
    input_          = con["parameters"]["inputs"];
    output_         = con["parameters"]["outputs"];
    hidden_         = con["parameters"]["hiddens"];
    mutate_wires_   = con["parameters"]["mutate-wires"];
    mutate_weights_ = con["parameters"]["mutate-weights"];
    mutate_acfns_   = con["parameters"]["mutate-acfns"];
    mutate_cppn_    = con["parameters"]["mutate-cppn"];
    genome_config_  = con["parameters"]["genome-params"];

    in_sense_  = con["input-tags"]["in-sense"];
    out_sense_ = con["output-tags"]["out-sense"];

    life::configuration c = my_cppn_.publish_configuration();
    // yucky
    internal_cppn_input_tag    = c["input-tags"]["in-sense"];
    internal_cppn_output_tag   = c["output-tags"]["out-sense"];

    c["parameters"]["inputs"]  = 2;
    c["parameters"]["outputs"] = 1;
    c["parameters"]["hiddens"] = cppn_hidden_;
    c["parameters"]["recurr"] = cppn_recurr_;
    my_cppn_.configure(c);

    genome_.configure(genome_config_);
    genome_.generate(200);
    buffer_ = std::vector(input_ + output_ + hidden_, 0.);
    seed_gates_(4);
  }

  void           seed_gates(size_t);
  void           mutate();
  void           input(std::string, life::signal);
  life::signal   output(std::string);
  void           tick();
  life::encoding get_encoding() const { return genome_; }
  void           set_encoding(life::encoding e)
  {
    genome_ = e;
    compute_gates_();
  }
  life::encoding parse_encoding(std::string);
};
