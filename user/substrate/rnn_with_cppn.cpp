
#include "rnn_with_cppn.h"
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

ded::encoding
    rnn_with_cppn::parseEncoding(std::string s)
{
  std::smatch    m;
  ded::encoding e;
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
    rnn_with_cppn::reconstruct_weights_()
{
  // auto norm = [](auto x, auto scale) { return (x * 4 * ded::utilities::PI) / scale; };

  for (auto i : rv::iota(0, input_ + recurr_))
    for (auto j : rv::iota(0, recurr_ + output_))
    {
      internal_cppn_.input(internal_cppn_input_tag,
                           std::vector<double>{ source_x_ + i * offset_,
                                                source_y_ + j * offset_ });
      internal_cppn_.tick();
      auto v = std::get<std::vector<double>>(
          internal_cppn_.output(internal_cppn_output_tag));
      genome_[i * (recurr_ + output_) + j] = (v[0] + 1) * 50;
    }
}

void
    rnn_with_cppn::reset()
{
  for (auto &b : buffer_) b = 0;
}

void
    rnn_with_cppn::mutate()
{
  internal_cppn_.mutate();
  reconstruct_weights_();
}

void
    rnn_with_cppn::input(std::string n, ded::signal s)
{
  if (n == in_sense_)
  {
    auto in = std::get<std::vector<double>>(s);
    rs::copy(in, rs::begin(buffer_));
  }
}

ded::signal
    rnn_with_cppn::output(std::string n)
{
  if (n == out_sense_)
    return buffer_ | rs::copy |
           ra::slice(input_ + recurr_, input_ + recurr_ + output_);

  return {};
}

void
    rnn_with_cppn::tick()
{
  decltype(buffer_) temp_buffer_;

  auto in_recs = buffer_ | rs::copy |
                 ra::slice(size_t{ 0 }, input_ + recurr_);

  for (auto weights : genome_ | rv::chunk(input_ + recurr_))
    temp_buffer_.push_back(rs::inner_product(
        weights,
        in_recs,
        0.,
        [](int w, auto i) { return (((w % 100) - 50) / 50.) * i; },
        std::plus{}));

  rs::copy(temp_buffer_, rs::begin(buffer_) + input_);
}

/*
// for debugging purposes
void
    rnn::print()
{
  for (auto &node : nodes_)
  {
    std::cout << "af: " << node.activation_function << " ws: ";
    for (auto &w : node.in_node)
      std::cout << "{" << w.first << "," << w.second << "} ";
    std::cout << std::endl;
  }
}
*/

