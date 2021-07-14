#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <vector>

class det_logic
{

  ded::concepts::Encoding  genome_;
  ded::specs::EncodingSpec genome_spec_ = genome_.publishConfiguration();
  std::regex               encoding_parser_{ R"(([^:]+):)" };

  long input_  = 10;
  long output_ = 10;
  long hidden_ = 10;

  std::vector<double> buffer_;

  std::vector<long> codon_{ 42, 214 };
  long              gene_length_ = 28;

  std::vector<long> logic_, ins_, outs_;

  void compute_gate_();

public:
  void
      configuration(ded::specs::SubstrateSpec &es)
  {

    es.parameter("inputs", input_);
    es.parameter("outputs", output_);
    es.parameter("hiddens", hidden_);

    es.input("ins", "<double,inputs>");
    es.output("outs", "<double,outputs>");

    es.encoding("genome", genome_spec_);
    genome_.configure(genome_spec_);

    genome_.generate(28);
    genome_[0] = 42;
    genome_[0] = 214;

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
    genome_.set(e.get() | ra::slice(0, 28));
    compute_gate_();
  }
  ded::concepts::Encoding parseEncoding(std::string)
  {
    return {};
  }
};
