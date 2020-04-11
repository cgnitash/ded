#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <vector>

class markov_brain_indirect
{

  std::string in_sense_  = "<double,inputs>";
  std::string out_sense_ = "<double,outputs>";

  ded::concepts::Encoding  genome_;
  ded::specs::EncodingSpec genome_spec_ = genome_.publishConfiguration();
  std::regex               encoding_parser_{ R"(([^:]+):)" };

  ded::specs::SubstrateSpec indirect_spec_ =
      ded::defaultSubstrateSpec("null_substrate");

  long input_      = 10;
  long output_     = 10;
  long hidden_     = 10;
  bool individual_ = false;
  bool irregular_  = false;

  std::vector<double> buffer_;

  std::vector<long> codon_{ 7, 14 };

  long indirect_gene_length_{};

  struct gate
  {
    std::vector<long> logic_, ins_, outs_;
  };

  std::vector<gate> gates_;

  bool                             gates_are_computed_ = false;
  void                             computeGates_();
  ded::concepts::Substrate         constructIndirect_(int pos);
  std::vector<std::vector<double>> getRegularInputs(int pos);
  std::vector<std::vector<double>> getIrregularInputs(int pos);

public:
  markov_brain_indirect()
  {
    configure(publishConfiguration());
  }

  ded::specs::SubstrateSpec
      publishConfiguration()
  {
    ded::specs::SubstrateSpec es;

    es.bindParameter("inputs", input_);
    es.bindParameter("outputs", output_);
    es.bindParameter("hiddens", hidden_);
    es.bindParameter("individual", individual_);
    es.bindParameter("irregular", irregular_);

    es.bindInput("in_sense", "<double,inputs>");
    es.bindOutput("out_sense", "<double,outputs>");

    es.bindEncoding("genome", genome_spec_);

    es.bindSubstrate("indirect",
                     indirect_spec_,
                     { { "indirect_input", "<double,2>" } },
                     { { "indirect_output", "<double,1>" } });

    return es;
  }

  void
      configure(ded::specs::SubstrateSpec es)
  {

    es.configureParameter("inputs", input_);
    es.configureParameter("outputs", output_);
    es.configureParameter("hiddens", hidden_);
    es.configureParameter("individual", individual_);
    es.configureParameter("irregular", irregular_);

    es.configureInput("in_sense", in_sense_);

    es.configureOutput("out_sense", out_sense_);

    es.configureSubstrate("indirect", indirect_spec_);

    indirect_gene_length_ =
        ded::makeSubstrate(indirect_spec_).getEncoding().size();

    es.configureEncoding("genome", genome_spec_);
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
    computeGates_();
  }
  ded::concepts::Encoding parseEncoding(std::string);
};
