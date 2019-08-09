#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class markov_brain {

  std::string in_sense_  = "<double,inputs>";
  std::string out_sense_ = "<double,outputs>";

  ded::concepts::Encoding genome_;
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  long input_  = 10;
  long output_ = 10;
  long hidden_ = 10;

  //ded::specs::EntitySpec genome_spec_;
  	
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
  ded::specs::EntitySpec publish_configuration()
  {
    ded::specs::EntitySpec es;

	es.bindParameter("inputs",input_);
	es.bindParameter("outputs",output_);
	es.bindParameter("hiddens",hidden_);

    es.bindIinput("in_sense","<double,inputs>");
    es.bindOutput("out_sense","<double,outputs>");

    return es;
  }
  void configure(ded::specs::EntitySpec es)
  {
  
	es.configureParameter("inputs",input_);
	es.configureParameter("outputs",output_);
	es.configureParameter("hiddens",hidden_);

    es.configureInput("in_sense", in_sense_);

    es.configureOutput("out_sense", out_sense_);

	  // MUST BE ADDRESSED
  //genome_config_ = con["parameters"]["genome-params"];
    //genome_.configure(genome_config_);
    //es.configureEntity("genome", genome_spec_ );

    genome_.generate(500);
    buffer_ = std::vector(input_ + output_ + hidden_, 0.);
    seed_gates_(4);
  }

  void           reset();
  void           mutate();
  void           input(std::string, ded::concepts::Signal);
  ded::concepts::Signal   output(std::string);
  void           tick();
  ded::concepts::Encoding get_encoding() const { return genome_; }
  void           set_encoding(ded::concepts::Encoding e)
  {
    genome_ = e;
    compute_gates_();
  }
  ded::concepts::Encoding parse_encoding(std::string);
};
