# pragma once

#include"../../components.h"

#include <vector>
#include <algorithm>

class markov_brain {

  std::string in_sense_ = "in-sense,A<double,inputs>";
  std::string out_sense_ = "out-sense,A<double,outputs>";

  life::encoding genome_;
  std::regex encoding_parser_{R"(([^:]+):)"};

  size_t input_ = 10;
  size_t output_ = 10;
  size_t hidden_ = 10;
  
  life::configuration genome_config_;

  std::vector<double> buffer_;

  std::vector<long> codon_{7, 14};
  long gene_length_ = 28;

  struct gate {
    std::vector<long> logic_, ins_, outs_;
  };

  std::vector<gate> gates_;

  bool gates_are_computed_ = false;
  void compute_gates_();
  void seed_gates_(size_t = 1);

public:
  markov_brain() { configure(publish_configuration()); }
  life::configuration publish_configuration() {
    life::configuration con;
    con["parameters"]["inputs"] = input_;
    con["parameters"]["outputs"] = output_;
    con["parameters"]["hiddens"] = hidden_;
    con["parameters"]["genome-params"] = genome_config_;

	con["input-tags"]["in-sense"] = in_sense_;
	con["output-tags"]["out-sense"] = out_sense_;

    return con;
  }
  void configure(life::configuration con) {
    input_ = con["parameters"]["inputs"];
    output_ = con["parameters"]["outputs"];
    hidden_ = con["parameters"]["hiddens"];
    genome_config_ = con["parameters"]["genome-params"];

    in_sense_ = con["input-tags"]["in-sense"];
    out_sense_ = con["output-tags"]["out-sense"]; 

    genome_.configure(genome_config_);
    genome_.generate(500);
    buffer_ = std::vector(input_ + output_ + hidden_, 0.);
    seed_gates_(4);
  }

  void mutate();
  void input(std::string, life::signal);
  life::signal output(std::string);
  void tick();
  life::encoding get_encoding() const { return genome_; }
  void set_encoding(life::encoding e) {
    genome_ = e;
    compute_gates_();
  }
  life::encoding parse_encoding(std::string);
};
