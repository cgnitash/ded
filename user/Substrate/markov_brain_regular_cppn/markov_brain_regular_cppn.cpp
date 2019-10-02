
#include "markov_brain_regular_cppn.h"

#include <algorithm>
#include <vector>

ded::concepts::Encoding
    markov_brain_regular_cppn::parseEncoding(std::string s)
{
  ded::concepts::Encoding e;
  for (std::sregex_iterator end,
       i(std::begin(s), std::end(s), encoding_parser_);
       i != end;
       i++)
  {
    auto site = (*i)[1].str();
    if (!site.empty())
    {
      e.push_back(std::stol(site));
    }
  }
  return e;
}

void
    markov_brain_regular_cppn::reset()
{
  for (auto &b : buffer_)
    b = 0;
}

void
    markov_brain_regular_cppn::mutate()
{

  genome_.pointDelete();
  genome_.pointInsert();
  genome_.pointMutate();
  genome_.copyChunk();
  genome_.delChunk();
  gates_are_computed_ = false;
  buffer_             = std::vector(input_ + output_ + hidden_, 0.);
}

void
    markov_brain_regular_cppn::input(std::string n, ded::concepts::Signal s)
{
  if (n == in_sense_)
  {
    auto v = std::get<std::vector<double>>(s);
    for (auto i{ 0u }; i < input_; i++)
      buffer_[i] = ded::utilities::Bit(v[i]);
  }
}

ded::concepts::Signal
    markov_brain_regular_cppn::output(std::string n)
{

  if (n == out_sense_)
  {
    return buffer_ | rs::copy | ra::slice(input_, input_ + output_);
  }

  return {};
}

void
    markov_brain_regular_cppn::tick()
{

  if (!gates_are_computed_)
  {
    compute_gates_();
    gates_are_computed_ = true;
  }

  std::vector out_buffer(buffer_.size(), 0.);

  for (auto &g : gates_)
  {
    auto in = 0;
    for (auto &i : g.ins_)
      in = in * 2 + buffer_[i];
    auto out = g.logic_[in];
    for (auto &i : g.outs_)
      out_buffer[i] += out;
  }
  buffer_ = out_buffer | rs::move |
            ra::transform([](auto i) { return ded::utilities::Bit(i); });
}

void
    markov_brain_regular_cppn::compute_gates_()
{
  auto                    cppn = ded::makeSubstrate(cppn_spec_);
  ded::concepts::Encoding e;
  e.set({ std::begin(genome_), std::begin(genome_) + cppn_gene_length_ });
  cppn.setEncoding(e);

  gates_.clear();
  auto addresses = input_ + output_ + hidden_;
  for (auto pos{ std::begin(genome_) }; pos < std::end(genome_) - gene_length_;
       pos++)
  {
    // find the next codon
    pos = std::search(pos,
                      std::end(genome_) - gene_length_,
                      std::begin(codon_),
                      std::end(codon_));
    if (pos != std::end(genome_) - gene_length_)
    {
      // convert the gene into gate
      gate g;
      // translate input wires
      auto in = *(pos + 2) % 4 + 1;
      for (auto i : rv::iota(0, in))
        g.ins_.push_back(*(pos + 3 + i) % addresses);

      // translate output wires
      auto out = *(pos + 7) % 4 + 1;
      for (auto i : rv::iota(0, out))
        g.outs_.push_back(*(pos + 8 + i) % addresses);

      // translate logic
      auto x_origin = *(pos + 12) % 6 - 3.14;
      auto y_origin = *(pos + 13) % 6 - 3.14;
      auto x_offset = *(pos + 14) % 6 - 3.14;
      auto y_offset = *(pos + 15) % 6 - 3.14;
      for (auto i : rv::iota(0, 16))
      {
        cppn.input("cppn_input",
                   std::vector{ x_origin + (i / 4 * x_offset),
                                y_origin + (i % 4 * y_offset) });
        auto co = std::get<std::vector<double>>(cppn.output("cppn_output"));
        cppn.tick();
        g.logic_.push_back(ded::utilities::Bit(co[0]));
      }

      gates_.push_back(g);
    }
  }
}
