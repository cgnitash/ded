
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
    std::cout
        << "Error: entity-markov2in1out must get an input range of the specified size\n";
    exit(1);
  }


  // must convert double inputs to 1s and 0s
  buffer_ = v | util::rv3::copy |
            util::rv3::action::transform([](auto i) { return util::Bit(i); });
  //  std::transform(std::begin(v), std::end(v), std::begin(buffer_),
  //               [](auto const value) { return util::Bit(value); });
}

life::signal markov2in1out::output() {
  // life::signal v =
  return buffer_ | util::rv3::copy |
         util::rv3::action::slice(input_, input_ + output_);

  //	  std::copy_n(std::begin(buffer_) + input_, output_,
  //std::back_inserter(v));
}

void markov2in1out::tick() {

  //std::cout << buffer_.size() << std::endl;
  // temporary buffer, since markov-brain logic updates must be synced
  std::vector out_buffer(buffer_.size(), 0u);

  // sum up all logic outputs
  for (auto &g : gates_)
    out_buffer[g.out_] += g.logic_[buffer_[g.in_1_] * 2 + buffer_[g.in_2_]];

  // make the buffer 1s and 0s again
  std::transform(std::begin(out_buffer), std::end(out_buffer),
                 std::begin(buffer_),
                 [](auto const value) { return util::Bit(value); });
}

void markov2in1out::seed_gates(size_t n) {

  for (auto i{0u}; i < n; i++) {
    auto pos = std::rand() % (genome_.size() - 1);
    genome_[pos] = 2;
    genome_[pos + 1] = 8;
  }
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
