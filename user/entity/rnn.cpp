
#include "rnn.h"
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
    rnn::parse_encoding(std::string s)
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
    rnn::reset()
{
  for (auto &b : buffer_) b = 0;
}

void
    rnn::mutate()
{
  // only point mutations since size of encoding can't change
  genome_.point_mutate();
}

void
    rnn::input(std::string n, life::signal s)
{
  if (n == in_sense_)
  {
    auto in = std::get<std::vector<double>>(s);
    ranges::copy(in, ranges::begin(buffer_));
  }
}

life::signal
    rnn::output(std::string n)
{
  if (n == out_sense_)
    return buffer_ | ranges::copy |
           ranges::action::slice(input_ + recurr_, input_ + recurr_ + output_);

  return {};
}

void
    rnn::tick()
{
  decltype(buffer_) temp_buffer_;

  auto in_recs = buffer_ | ranges::copy |
                 ranges::action::slice(size_t{ 0 }, input_ + recurr_);

  for (auto weights : genome_ | ranges::view::chunk(input_ + recurr_))
    temp_buffer_.push_back(ranges::inner_product(
        weights,
        in_recs,
        0.,
        [](int w, auto i) { return (((w % 100) - 50) / 50.) * i; },
        std::plus{}));

  ranges::copy(temp_buffer_, ranges::begin(buffer_) + input_);
}

