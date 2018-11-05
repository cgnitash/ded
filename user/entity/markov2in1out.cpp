
#include"markov2in1out.h"
#include"../../core/utilities.h"

#include <vector>
#include <algorithm>
#include <regex>

void markov2in1out::mutate() {

  // go nuts with all kinds of mutations
  life::all_mutations(genome_);
  compute_gates_();
}

void markov2in1out::input(life::signal v) {

  if (v.size() != input_) {
    std::cout << "Error: entity-markov2in1out must get an input range of the "
                 "specified size\n";
    exit(1);
  }

  // must convert double inputs to 1s and 0s, overwrite only input range of
  // buffer_
  for (auto i{0u}; i < v.size(); i++)
    buffer_[i] = util::Bit(v[i]);

}

life::signal markov2in1out::output() {

  return buffer_ | util::rv3::copy |
         util::rv3::action::slice(input_, input_ + output_);
}

void markov2in1out::tick() {

  std::vector out_buffer(buffer_.size(), 0.);

  // sum up all logic outputs
  for (auto &g : gates_)
    out_buffer[g.out_] += g.logic_[buffer_[g.in_1_] * 2 + buffer_[g.in_2_]];

  // make the buffer 1s and 0s again
  buffer_ = out_buffer | util::rv3::copy |
            util::rv3::action::transform([](auto i) { return util::Bit(i); });

}

void markov2in1out::seed_gates(size_t n) {

  util::repeat(n, [&] {
    auto pos = std::rand() % (genome_.size() - 1);
    genome_[pos] = 2;
    genome_[pos + 1] = 8;
  });
}


void markov2in1out::compute_gates_() {

  gates_.clear();
  auto addresses = input_ + output_ + hidden_;
  std::vector codon{2, 8};
  for (auto pos{std::begin(genome_)}; pos < std::end(genome_) - 10; pos++) {
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
}
