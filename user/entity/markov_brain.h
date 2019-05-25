#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class markov_brain {

  std::string in_sense_  = "in-sense,A<double,inputs>";
  std::string out_sense_ = "out-sense,A<double,outputs>";

  life::encoding genome_;
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  long input_  = 10;
  long output_ = 10;
  long hidden_ = 10;

  life::configuration genome_config_;

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
  markov_brain() { configure(publish_configuration()); }
  life::entity_spec publish_configuration()
  {
    life::entity_spec es;
    //con["parameters"]["inputs"]        = input_;
    //con["parameters"]["outputs"]       = output_;
    //con["parameters"]["hiddens"]       = hidden_;
    //con["parameters"]["genome-params"] = genome_config_;

	es.bind_parameter("inputs",input_);
	es.bind_parameter("outputs",output_);
	es.bind_parameter("hiddens",hidden_);

    es.bind_input("in-sense", in_sense_);
    es.bind_output("out-sense", out_sense_);
    //con["input-tags"]["in-sense"]   = in_sense_;
    //con["output-tags"]["out-sense"] = out_sense_;

    return es;
  }
  void configure(life::entity_spec es)
  {
  //  input_         = con["parameters"]["inputs"];
  //  output_        = con["parameters"]["outputs"];
  //  hidden_        = con["parameters"]["hiddens"];
  
	  // MUST BE ADDRESSED
  //  genome_config_ = con["parameters"]["genome-params"];

	es.configure_parameter("inputs",input_);
	es.configure_parameter("outputs",output_);
	es.configure_parameter("hiddens",hidden_);

    es.configure_input("in-sense", in_sense_);

    es.configure_output("out-sense", out_sense_);
    //in_sense_  = con["input-tags"]["in-sense"];
    //out_sense_ = con["output-tags"]["out-sense"];

    genome_.configure(genome_config_);
    genome_.generate(500);
    buffer_ = std::vector(input_ + output_ + hidden_, 0.);
    seed_gates_(4);
  }

  void           reset();
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
