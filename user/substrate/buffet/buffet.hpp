#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <vector>

class buffet
{

  ded::concepts::Encoding  genome_;
  ded::specs::EncodingSpec genome_spec_ = genome_.publishConfiguration();
  std::regex               encoding_parser_{ R"(([^:]+):)" };

  long input_  = 10;
  long output_ = 10;
  long hidden_ = 10;

  std::vector<double> buffer_;

  std::vector<ded::concepts::Substrate>  gates_;
  std::vector<ded::specs::SubstrateSpec> gate_specs_;

  std::vector<long> codon_{ 7, 14 };

  struct GateEncodings
  {
    long                      gene_length;
    std::vector<long>         start_codon;
    ded::specs::SubstrateSpec gate_spec;
  };

  std::vector<GateEncodings> gate_encodings_;

  bool gates_are_computed_ = false;
  void compute_gates_();

public:
  void
      configuration(ded::specs::SubstrateSpec &es)
  {

    es.parameter("inputs", input_);
    es.parameter("outputs", output_);
    es.parameter("hiddens", hidden_);

    es.input("in_layer", "<double,inputs>");
    es.output("out_layer", "<double,outputs>");

    es.encoding("genome", genome_spec_);
    genome_.configure(genome_spec_);

    es.nestedSubstrateVector("gates", gate_specs_);

    genome_.generate(500);
    genome_.seedCodons(codon_, 4);
    for (auto &gate_spec : gate_specs_)
    {
      auto gate = ded::makeSubstrate(gate_spec);
      gates_.push_back(gate);
      auto ge = gate.getEncoding();
      gate_encodings_.push_back({ static_cast<long>(gate.getEncoding().size()),
                                  ge | rv::take(2) | rs::to<std::vector<long>>,
                                  gate_spec });
    }

    for (auto &gate_encoding : gate_encodings_)
    {
      genome_.seedCodons(gate_encoding.start_codon, 4);
    }

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
  ded::concepts::Encoding parseEncoding(std::string)
  {
    return {};
  }
};
