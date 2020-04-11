
#include "markov_brain.hpp"

#include <algorithm>
#include <vector>

ded::concepts::Encoding
    markov_brain::parseEncoding(std::string s)
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
    markov_brain::reset()
{
  for (auto &b : buffer_)
    b = 0;
}
void
    markov_brain::mutate()
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
    markov_brain::input(std::string n, ded::concepts::Signal s)
{
  if (n == "in_sense")
  {
    auto v = std::any_cast<std::vector<double>>(s);
    if (static_cast<long>(v.size()) != input_)
    {
      std::cout
          << "Impl-Error: substrate-markovbrain must get an input range of the "
             "specified size\n";
      exit(1);
    }
    for (auto i{ 0u }; i < input_; i++)
      buffer_[i] = ded::utilities::Bit(v[i]);
  }
  else
  {
    std::cout << "Impl-Error: substrate-markovbrain cannot handle this "
                 "name-signal pair in input \n";
    exit(1);
  }
}

ded::concepts::Signal
    markov_brain::output(std::string n)
{

  if (n ==  "out_sense")
  {
    return buffer_ | rs::copy | ra::slice(input_, input_ + output_);
  }
  else
  {
    std::cout << "Impl-Error: substrate-markovbrain cannot handle this "
                 "name-signal pair in output \n";
    exit(1);
  }
}

void
    markov_brain::tick()
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

/*
void
    markov_brain::seed_gates_(size_t n)
{

        ded::utilities::repeat(n, [&] {
    auto pos = std::rand() % (genome_.size() - 1);
    std::copy(std::begin(codon_), std::end(codon_), std::begin(genome_) + pos);
  });
  gates_are_computed_ = false;
}
*/

void
    markov_brain::compute_gates_()
{

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
      for (auto i : rv::iota(0, 16))
        g.logic_.push_back(*(pos + 13 + i) % 2);

      gates_.push_back(g);
    }
  }
}
