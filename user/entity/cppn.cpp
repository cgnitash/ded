
#include "cppn.h"
#include "../../core/utilities.h"

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

life::encoding
    cppn::parse_encoding(std::string s)
{
  std::smatch    m;
  life::encoding e;
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
  for (size_t i : ranges::view::iota(0, output_ + hidden_))
  {
    Node node;
    node.activation_function = genome_[9 * i] % 6;
    for (auto j = 0; j < 4; j++)
      node.in_node[genome_[9 * i + j * 2 + 1] % (input_ + i + 1)] =
          (genome_[9 * i + j * 2 + 2] % 1000) / 1000.0;
    nodes_.push_back(node);
  }
}

void
    cppn::mutate()
{

  // only point mutations since size of encoding can't change
  genome_.point_mutate();
  gates_are_computed_ = false;
}

void
    cppn::input(std::string n, life::signal s)
{
  if (n == in_sense_)
  {
    ins_ = std::get<std::vector<double>>(s);
    if (ins_.size() != input_)
    {
      std::cout << "Error: entity-cppn must get an input range of the "
                   "specified size\n";
      exit(1);
    }
  } else
  {
    std::cout << "Error: entity-cppn cannot handle this payload type \n";
    exit(1);
  }
}

life::signal
    cppn::output(std::string n)
{

  if (n == out_sense_)
  {
    return outs_;
  } else
  {
    std::cout << "Impl-Error: entity-cppn cannot handle this name-signal pair "
                 "in output \n";
    exit(1);
  }
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

  if (ins_.size() != input_)
  {
    std::cout
        << "Error: entity-cppn must get an input range of the specified size\n";
    exit(1);
  }
  auto results = ins_;

  // for each node in order
  for (auto &node : nodes_)
  {
    // sum up the incoming weighted-outputs from other nodes
    auto const sum = ranges::accumulate(
        node.in_node, 0.0, [&results](auto const total, auto const value) {
          return total + results[value.first] * value.second;
        });
    // fire the activation function
    results.push_back(activate(node.activation_function, sum));
  }

  auto res_size = results.size();
  // put the last output node results into the output buffer
  outs_ = results | ranges::move |
          ranges::action::slice(res_size - output_, res_size);
}

double
    cppn::activate(size_t c, double x)
{

  // TODO activation function set needs to be thought out
  // TODO  activation function set needs to be parameterised
  auto xp = c == 0 ? std::sin(x)
                   : c == 1 ? std::cos(x)
                            : c == 2 ? std::tan(x)
                                     : c == 3 ? std::sqrt(std::abs(x))
                                              : c == 4 ? std::fmod(x, 1) : x;
  return std::clamp(xp, -2 * util::PI, 2 * util::PI);
}

// for debugging purposes
void
    cppn::print()
{
  for (auto &node : nodes_)
  {
    std::cout << "af: " << node.activation_function << " ws: ";
    for (auto &w : node.in_node)
      std::cout << "{" << w.first << "," << w.second << "} ";
    std::cout << std::endl;
  }
}
