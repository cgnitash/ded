
#include "markov2in1out.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <regex>
#include <vector>

ded::encoding
    markov2in1out::parseEncoding(std::string s)
{
  ded::encoding e;
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


void
    markov2in1out::reset()
{
  for (auto &b : buffer_) b = 0;
}

void
    markov2in1out::mutate()
{

  // go nuts with all kinds of mutations
  genome_.all_mutations();
  gates_are_computed_ = false;
  buffer_             = std::vector(input_ + output_ + hidden_, 0.);
}

void
    markov2in1out::input(std::string n, ded::signal s)
{

  if (n == in_sense_)
  {
    auto v = std::get<std::vector<double>>(s);
    if (v.size() != input_)
    {
      std::cout << "Error: substrate-markov2in1out must get an input range of the "
                   "specified size\n";
      exit(1);
    }
    for (auto i{ 0u }; i < v.size(); i++) buffer_[i] = ded::utilities::Bit(v[i]);

  } else
  {
    std::cout
        << "Error: substrate-markov2in1out cannot handle this payload type \n";
    exit(1);
  }
}

ded::signal
    markov2in1out::output(std::string n)
{

  if (n == out_sense_)
  {
    return buffer_ | rs::copy |
           ra::slice(input_, input_ + output_);
  } else
  {
    std::cout << "Impl-Error: substrate-markov2inIout cannot handle this "
                 "name-signal pair in output \n";
    exit(1);
  }
}

void
    markov2in1out::tick()
{

  if (!gates_are_computed_)
  {
    compute_gates_();
    gates_are_computed_ = true;
  }

  std::vector out_buffer(buffer_.size(), 0.);

  // sum up all logic outputs
  for (auto &g : gates_)
    out_buffer[g.out_] += g.logic_[buffer_[g.in_1_] * 2 + buffer_[g.in_2_]];

  // make the buffer 1s and 0s again
  buffer_ = out_buffer | rs::copy |
            ra::transform([](auto i) { return ded::utilities::Bit(i); });
}

void
    markov2in1out::seed_gates_(size_t n)
{

  ded::utilities::repeat(n, [&] {
    auto pos         = std::rand() % (genome_.size() - 1);
    genome_[pos]     = 2;
    genome_[pos + 1] = 8;
  });
  gates_are_computed_ = false;
}

void
    markov2in1out::compute_gates_()
{

  gates_.clear();
  auto        addresses = input_ + output_ + hidden_;
  std::vector codon{ 2, 8 };
  for (auto pos{ std::begin(genome_) }; pos < std::end(genome_) - 10; pos++)
  {
    // find the next codon
    pos = std::search(
        pos, std::end(genome_) - 10, std::begin(codon), std::end(codon));
    if (pos != std::end(genome_) - 10)
    {
      // convert the gene into gate
      gate g{
        *(pos + 2) % addresses,
        *(pos + 3) % addresses,
        *(pos + 4) % addresses,
        { *(pos + 5) % 2, *(pos + 6) % 2, *(pos + 7) % 2, *(pos + 8) % 2 }
      };
      gates_.push_back(g);
    }
  }
}
