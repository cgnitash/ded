#pragma once

#include "../../components.h"

#include <algorithm>
#include <vector>

class det_logic {

  std::string in_sense_  = "<double,inputs>";
  std::string out_sense_ = "<double,outputs>";

  ded::concepts::Encoding genome_;
  ded::specs::EncodingSpec genome_spec_ = genome_.publishConfiguration();
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  long input_  = 10;
  long output_ = 10;
  long hidden_ = 10;

  //ded::specs::SubstrateSpec genome_spec_;
  	
  
  std::vector<double> buffer_;

  std::vector<long> codon_{ 42, 214 };
  long              gene_length_ = 28;

  std::vector<long> logic_, ins_, outs_;

  //std::vector<gate> gates_;

  void compute_gate_();
  //void seed_gates_(size_t = 1);

public:
  det_logic() { configure(publishConfiguration()); }

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
	es.configureParameter("hiddens",hidden_);

    es.configureInput("in_sense", in_sense_);

    es.configureOutput("out_sense", out_sense_);

	es.configureEncoding("genome", genome_spec_);
	genome_.configure(genome_spec_);

    genome_.generate(28);
	genome_[0] = 42; 
	genome_[0] = 214; 

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
    genome_.set(e.get()| ra::slice(0,28));
    compute_gate_();
  }
  ded::concepts::Encoding parseEncoding(std::string)
  {
    return {};
  }
};
