
#pragma once

#include "../../components.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <utility>
#include <vector>

class cppn {

private:
  std::string in_sense_  = "<double,inputs>";
  std::string out_sense_ = "<double,outputs>";


  struct Node
  {
    long                   activation_function;
    std::map<long, double> in_node;
  };
  std::vector<Node> nodes_;

  long input_  = 2;
  long output_ = 1;
  long hidden_ = 0;

  ded::concepts::Encoding genome_;
  ded::specs::EncodingSpec genome_spec_ = genome_.publishConfiguration();
  std::regex     encoding_parser_{ R"(([^:]+):)" };

  std::vector<double> ins_;
  std::vector<double> hiddens_;
  std::vector<double> outs_;

  double activate(size_t, double);
  bool   gates_are_computed_ = false;
  void   compute_nodes_();

public:
  cppn() { configure(publishConfiguration()); }

  ded::specs::SubstrateSpec publishConfiguration()
  {
    ded::specs::SubstrateSpec es;
	es.bindParameter("inputs",input_);
	es.bindParameter("outputs",output_);
	es.bindParameter("hiddens",hidden_);

    es.bindInput("ins","<double,inputs>");
    es.bindOutput("outs","<double,outputs>");

	es.bindEncoding("genome", genome_spec_);
    return es;
  }

  void configure(ded::specs::SubstrateSpec es)
  {
  
	es.configureParameter("inputs",input_);
	es.configureParameter("outputs",output_);
	es.configureParameter("hiddens",hidden_);

    es.configureInput("ins", in_sense_);

    es.configureOutput("outs", out_sense_);

	es.configureEncoding("genome", genome_spec_);
	genome_.configure(genome_spec_);

        genome_.generate(9 * (output_ + hidden_));

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
    compute_nodes_();
  }
  ded::concepts::Encoding parseEncoding(std::string);
};
