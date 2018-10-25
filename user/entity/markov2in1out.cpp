
#include"markov2in1out.h"
#include"../../core/utilities.h"

#include <vector>
#include <algorithm>
#include <regex>

void markov2in1out::mutate() {

  life::all_mutations(genome_);
  gates_valid_ = false;
  gates_.clear();
  buffer_ = std::vector<long>(input_ + output_ + hidden_, 0);
}

void markov2in1out::input(life::signal v) {
  // must convert double inputs to 1s and 0s 
  std::transform(std::begin(v), std::begin(v) + input_,
                 std::back_inserter(buffer_),
                 [](auto const value) { return util::Bit(value); });
}

life::signal markov2in1out::output() {
  life::signal v;
  std::copy_n(std::begin(buffer_) + input_, output_, std::back_inserter(v));
  return v;
}

void markov2in1out::tick() {

  if (!gates_valid_)
    compute_gates_();

  std::vector out_buffer(buffer_.size(), 0l);

  // sum up all logic outputs
  for (auto &g : gates_)
    out_buffer[g.out_] += g.logic_[buffer_[g.in_1_] * 2 + buffer_[g.in_2_]];

  // make the buffer 1s and 0s again
  std::transform(std::begin(out_buffer), std::end(out_buffer),
                 std::begin(buffer_),
                 [](auto const value) { return util::Bit(value); });
}

void markov2in1out::seed_gates(long n) {
  for (int i = 0; i < n; i++) {
    auto pos = std::rand() % (genome_.size() - 1);
    genome_[pos] = 2;
    genome_[pos + 1] = 8;
  }
}

void markov2in1out::compute_gates_() {

  gates_.clear();
  auto addresses = input_ + output_ + hidden_;
  std::vector codon{2, 8};
  for (auto pos = std::begin(genome_); pos < std::end(genome_) - 10; pos++) {
    // find the next codon
    pos = std::search(pos, std::end(genome_) - 10, std::begin(codon),
                      std::end(codon));
    if (pos != std::end(genome_) - 10) {
      // convert the gene into gate
      gate g{*(pos + 2) % addresses,
             *(pos + 3) % addresses,
             *(pos + 4) % addresses,
             {*(pos + 5) % 2, *(pos + 6) % 2, *(pos + 7) % 2, *(pos + 8) % 2}};
      gates_.push_back(g);
    }
  }
  gates_valid_ = true;
}
