
#include "cppn.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <utility>
#include <vector>

ded::concepts::Encoding
    cppn::parseEncoding(std::string s)
{
  std::smatch    m;
  ded::concepts::Encoding e;
  for (std::sregex_iterator end,
       i(std::begin(s), std::end(s), encoding_parser_);
       i != end;
       i++)
  {
    auto site = (*i)[1].str();
    if (!site.empty()) e.push_back(std::stol(site));
  }
  return e;
}

void
    cppn::compute_nodes_()
{

  nodes_.clear();
  // create activation functions for all nodes other than inputs
  for (auto i : rv::iota(0, output_ + hidden_ ))
  {
    Node node;
    node.activation_function = genome_[9 * i] % 3;
    for (auto j = 0; j < 4; j++)
      node.in_node[genome_[9 * i + j * 2 + 1] % (input_  + i)] =
          (genome_[9 * i + j * 2 + 2] % 100) / 12.5 - 4.0;
    nodes_.push_back(node);
  }
}

void
    cppn::reset()
{
  if (hiddens_.empty()) hiddens_ = std::vector<double>(hidden_, 0.0);
}

void
    cppn::mutate()
{
  // only point mutations since size of encoding can't change
  genome_.pointMutate();
  gates_are_computed_ = false;
}

void
    cppn::input(std::string n, ded::concepts::Signal s)
{
  if (n == in_sense_) ins_ = std::get<std::vector<double>>(s);
}

ded::concepts::Signal
    cppn::output(std::string n)
{
  if (!gates_are_computed_) tick();

  if (n == out_sense_) return outs_;

  return {};
}

void
    cppn::tick()
{

  if (!gates_are_computed_)
  {
    compute_nodes_();
    gates_are_computed_ = true;
  }

  if (ins_.empty()) ins_ = std::vector<double>(input_, 0.0);
  if (hiddens_.empty()) hiddens_ = std::vector<double>(hidden_, 0.0);

  std::vector<double> results = rv::concat(ins_,hiddens_);

  // for each node in order
  for (auto &node : nodes_)
  {
    // sum up the incoming weighted-outputs from other nodes
    auto const sum = rs::accumulate(
        node.in_node, 0.0, [&results](auto const total, auto const value) {
          return total + results[value.first] * value.second;
        });
    // fire the activation function
    results.push_back(activate(node.activation_function, sum));
  }

  auto res_size = results.size();
  // put the last output node results into the output buffer
  outs_ = results | rs::copy |
          ra::slice(res_size - output_ - hidden_, res_size - hidden_);
  hiddens_ = results | rs::move |
          ra::slice(res_size - hidden_, res_size);
}

double
    cppn::activate(size_t c, double x)
{
  return c == 0 ? std::sin(x) : c == 1 ? std::tanh(x) : std::cos(x);
}

