
#include"markov_brain.h"
#include"../../core/utilities.h"

#include <vector>
#include <algorithm>

void markov_brain::mutate() {

  genome_.point_delete();
  genome_.point_insert();
  genome_.point_mutate();
  genome_.copy_chunk();
  genome_.del_chunk();
  compute_gates_();
  buffer_ = std::vector(input_ + output_ + hidden_, 0.);
}

void markov_brain::input(life::signal s) {
  if (auto vp = std::get_if<std::vector<double>>(&s)) {
    auto v = *vp;
    if (v.size() != input_) {
      std::cout << "Error: entity-markovbrain must get an input range of the "
                   "specified size\n";
      exit(1);
    }
    for (auto i{0u}; i < input_; i++)
      buffer_[i] = util::Bit(v[i]);
  } else {
    std::cout
        << "Error: entity-markovbrain cannot handle this payload type \n";
    exit(1);
  }
}

life::signal markov_brain::output() {

  return buffer_ | ranges::copy |
         ranges::action::slice(input_, input_ + output_);
}

void markov_brain::tick() {

  std::vector out_buffer(buffer_.size(), 0.);

  for (auto &g : gates_) {
    auto in = 0;
    for (auto &i : g.ins_)
      in = in * 2 + buffer_[i];
    auto out = g.logic_[in];
    for (auto &i : g.outs_) 
      out_buffer[i] += out;
  }
  buffer_ = out_buffer | ranges::move |
            ranges::action::transform([](auto i) { return util::Bit(i); });
}

void markov_brain::seed_gates_(size_t n) {

  util::repeat(n, [&] {
    auto pos = std::rand() % (genome_.size() - 1);
    genome_[pos] = 7;
    genome_[pos + 1] = 14;
  });
}

void markov_brain::compute_gates_() {

  gates_.clear();
  auto addresses = input_ + output_ + hidden_;
  std::vector codon{7, 14};
  auto gene_length = 28;
  for (auto pos{std::begin(genome_)}; pos < std::end(genome_) - gene_length;
       pos++) {
    // find the next codon
    pos = std::search(pos, std::end(genome_) - gene_length, std::begin(codon),
                      std::end(codon));
    if (pos != std::end(genome_) - gene_length) {
      // convert the gene into gate
      gate g;
	  // translate input wires
      auto in = *(pos + 2) % 4 + 1;
      for (auto i : ranges::view::iota(0u, in))
        g.ins_.push_back(*(pos + 3 + i) % addresses);

	  // translate output wires
      auto out = *(pos + 7) % 4 + 1;
      for (auto i : ranges::view::iota(0u, out))
        g.outs_.push_back(*(pos + 8 + i) % addresses);

	  // translate logic 
      for (auto i : ranges::view::iota(0u, 16))
        g.logic_.push_back(*(pos + 13 + i) % 2); 

      gates_.push_back(g);
    }
  }
}
