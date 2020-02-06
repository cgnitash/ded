
#include "det_logic.hpp"

#include <algorithm>
#include <vector>

/*
ded::concepts::Encoding
    det_logic::parseEncoding(std::string s)
{
  ded::concepts::Encoding e;
  for (std::sregex_iterator end,
       i(std::begin(s), std::end(s), encoding_parser_);
       i != end;
       i++)
  {
    auto site = (*i)[1].str();
    if (!site.empty()) { e.push_back(std::stol(site)); }
  }
  return e;
}
*/

void
    det_logic::reset()
{
  for (auto &b : buffer_)
    b = 0;
}

void
    det_logic::mutate()
{

  genome_.pointMutate();
  buffer_ = std::vector(input_ + output_ + hidden_, 0.);
  compute_gate_();
}

void
    det_logic::input(std::string n, ded::concepts::Signal s)
{
  if (n == in_sense_)
  {
    auto v = std::any_cast<std::vector<double>>(s);
    if (static_cast<long>(v.size()) != input_)
    {
      std::cout
          << "Impl-Error: substrate-det_logic must get an input range of the "
             "specified size\n";
      exit(1);
    }
    for (auto i{ 0u }; i < input_; i++)
      buffer_[i] = ded::utilities::Bit(v[i]);
  }
  else
  {
    std::cout << "Impl-Error: substrate-det_logic cannot handle this "
                 "name-signal pair in input \n";
    exit(1);
  }
}

ded::concepts::Signal
    det_logic::output(std::string n)
{

  if (n == out_sense_)
  {
    return buffer_ | rs::copy | ra::slice(input_, input_ + output_);
  }
  else
  {
    std::cout << "Impl-Error: substrate-det_logic cannot handle this "
                 "name-signal pair in output \n";
    exit(1);
  }
}

void
    det_logic::tick()
{
  std::vector out_buffer(buffer_.size(), 0.);

  auto in = 0;
  for (auto &i : ins_)
    in = in * 2 + buffer_[i];
  auto out = logic_[in];
  for (auto &i : outs_)
    out_buffer[i] += out;
  buffer_ = out_buffer | rs::move |
            ra::transform([](auto i) { return ded::utilities::Bit(i); });
}

void
    det_logic::compute_gate_()
{

  auto addresses = input_ + output_ + hidden_;
  // convert the gene into gate
  // translate input wires
  auto in = genome_[2] % 4 + 1;
  for (auto i : rv::iota(0, in))
    ins_.push_back(genome_[1 + i] % addresses);

  // translate output wires
  auto out = genome_[7] % 4 + 1;
  for (auto i : rv::iota(0, out))
    outs_.push_back(genome_[1 + i] % addresses);

  // translate logic
  for (auto i : rv::iota(0, 16))
    logic_.push_back(genome_[12 + i] % 2);
}
