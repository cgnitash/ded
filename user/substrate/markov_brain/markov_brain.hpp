#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <vector>

class markov_brain
{

public:

  void
      configuration(ded::specs::SubstrateSpec &es)
  {

    es.parameter("inputs", input_);
    es.parameter("outputs", output_);
    es.parameter("hiddens", hidden_);

    es.input("in_sense", "<double,inputs>");
    es.output("out_sense", "<double,outputs>");

    es.encoding("genome", genome_spec_);
    genome_.configure(genome_spec_);

    genome_.generate(500);
    genome_.seedCodons(codon_, 4);

    buffer_ = std::vector(input_ + output_ + hidden_, 0.);
  }

  void                  reset();
  void                  mutate();
  void                  input(std::string, ded::concepts::Signal);
  ded::concepts::Signal output(std::string);
  void                  tick();
  ded::concepts::Encoding
      getEncoding() const
  {
    return genome_;
  }
  void
      setEncoding(ded::concepts::Encoding e)
  {
    genome_ = e;
    compute_gates_();
  }
  ded::concepts::Encoding parseEncoding(std::string);

	private:

  ded::concepts::Encoding  genome_;
  ded::specs::EncodingSpec genome_spec_ = genome_.publishConfiguration();
  std::regex               encoding_parser_{ R"(([^:]+):)" };

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

};
