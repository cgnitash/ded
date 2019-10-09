#pragma once

#include "../../../components.h"

#include <algorithm>
#include <vector>

class markov_brain {

  std::string in_sense_  = "<double,inputs>";
  std::string out_sense_ = "<double,outputs>";

  ded::concepts::Encoding genome_;
  ded::specs::EncodingSpec genome_spec_ = genome_.publishConfiguration();
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  long input_  = 10;
  long output_ = 10;
  long hidden_ = 10;

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
  //void seed_gates_(size_t = 1);

public:
  markov_brain() { configure(publishConfiguration()); }

  ded::specs::SubstrateSpec publishConfiguration()
  {
    ded::specs::SubstrateSpec es;

	es.bindParameter("inputs",input_);
	es.bindParameter("outputs",output_);
	es.bindParameter("hiddens",hidden_);

    es.bindInput("in_sense","<double,inputs>");
    es.bindOutput("out_sense","<double,outputs>");

	es.bindEncoding("genome", genome_spec_);

    return es;
  }
  
  void configure(ded::specs::SubstrateSpec es)
  {
  
	es.configureParameter("inputs",input_);
	es.configureParameter("outputs",output_);
	es.configureParameter("hddens",hidden_);

    es.configureInput("in_sense", in_sense_);

    es.configureOutput("out_sense", out_sense_);

	es.configureEncoding("genome", genome_spec_);
	genome_.configure(genome_spec_);

    genome_.generate(500);
    genome_.seedCodons(codon_, 4);

    buffer_ = std::vector(input_ + output_ + hidden_, 0.);
  }

  void           reset();
  void           mutate();
  void           input(std::string, ded::concepts::Signal);
  ded::concepts::Signal   output(std::string);
  void           tick();
  ded::concepts::Encoding getEncoding() const { return genome_; }
  void           setEncoding(ded::concepts::Encoding e)
  {
    genome_ = e;
    compute_gates_();
  }
  ded::concepts::Encoding parseEncoding(std::string);
};
