
#include "buffet.hpp"

#include <algorithm>
#include <vector>

/*
ded::concepts::Encoding
    buffet::parseEncoding(std::string s)
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
*/

void
    buffet::reset()
{
  for (auto &b : buffer_)
    b = 0;
}
void
    buffet::mutate()
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
    buffet::input(std::string n, ded::concepts::Signal s)
{
  if (n == in_sense_)
  {
    auto v = std::get<std::vector<double>>(s);
    if (static_cast<long>(v.size()) != input_)
    {
      std::cout
          << "Impl-Error: substrate-buffet  must get an input range of the "
             "specified size\n";
      exit(1);
    }
    for (auto i{ 0u }; i < input_; i++)
      buffer_[i] = ded::utilities::Bit(v[i]);
  }
  else
  {
    std::cout << "Impl-Error: substrate-buffet cannot handle this "
                 "name-signal pair in input \n";
    exit(1);
  }
}

ded::concepts::Signal
    buffet::output(std::string n)
{

  if (n == out_sense_)
  {
    return buffer_ | rs::copy | ra::slice(input_, input_ + output_);
  }
  else
  {
    std::cout << "Impl-Error: substrate-buffet cannot handle this "
                 "name-signal pair in output \n";
    exit(1);
  }
}

void
    buffet::tick()
{

  if (!gates_are_computed_)
  {
    compute_gates_();
    gates_are_computed_ = true;
  }

  std::vector out_buffer(buffer_.size(), 0.);
  auto        input = buffer_ | rs::copy | ra::take(input_);
  for (auto &g : gates_)
  {
    g.input(in_sense_, input);
    g.tick();
    auto vec = std::get<std::vector<double>>(g.output(out_sense_));
    for (auto [i, v] : rv::enumerate(vec))
      out_buffer[i] += v;
  }
  buffer_ = out_buffer | rs::move |
            ra::transform([](auto i) { return ded::utilities::Bit(i); });
}

void
    buffet::compute_gates_()
{
  auto gates_copy = gates_;
  gates_.clear();
  for (auto &gate_encoding : gate_encodings_)
  {
    for (auto pos{ std::begin(genome_) };
         pos < std::end(genome_) - gate_encoding.gene_length;
         pos++)
    {
      // find the next gene
      pos = std::search(pos,
                        std::end(genome_) - gate_encoding.gene_length,
                        std::begin(gate_encoding.start_codon),
                        std::end(gate_encoding.start_codon));
      if (pos != std::end(genome_) - gate_encoding.gene_length)
      {
        auto gate = ded::makeSubstrate(gate_encoding.gate_spec);
        ded::concepts::Encoding e;
        e.set({ pos, pos + gate_encoding.gene_length });
        gate.setEncoding(e);
        gates_.push_back(gate);
      }
    }
  }
}
