
#include "markov_cppn.h"
#include "../../core/utilities.h"

#include <algorithm>
#include <vector>

void markov_cppn::mutate() {

  genome_.point_delete();
  genome_.point_insert();
  genome_.point_mutate();
  genome_.copy_chunk();
  genome_.del_chunk();

  if (mutate_acfns_) {
    auto cppn_con = my_cppn_.publish_configuration();
    auto e = my_cppn_.get_encoding();
    for (auto i{0u}; i < e.size(); i += 9)
      e[i * 9] = std::rand();
    my_cppn_.set_encoding(e);
  }

  if (mutate_wires_) {
    auto cppn_con = my_cppn_.publish_configuration();
    auto e = my_cppn_.get_encoding();
    for (auto i{0u}; i < e.size(); i += 9)
      for (auto j{0u}; j < 4; j++)
        e[i * 9 + j * 2 + 1] = std::rand();
    my_cppn_.set_encoding(e);
  }

  if (mutate_weights_) {
    auto cppn_con = my_cppn_.publish_configuration();
    auto e = my_cppn_.get_encoding();
    for (auto i{0u}; i < e.size(); i += 9)
      for (auto j{0u}; j < 4; j++)
        e[i * 9 + j * 2 + 2] = std::rand();
    my_cppn_.set_encoding(e);
  }

  compute_gates_();
  buffer_ = std::vector(input_ + output_ + hidden_, 0.);
}

void markov_cppn::input(life::signal s) {
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
    std::cout << "Error: entity-markovbrain cannot handle this payload type \n";
    exit(1);
  }
}

life::signal markov_cppn::output() {

  return buffer_ | ranges::copy |
         ranges::action::slice(input_, input_ + output_);
}

void markov_cppn::tick() {

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

void markov_cppn::seed_gates_(size_t n) {

  util::repeat(n, [&] {
    auto pos = std::rand() % (genome_.size() - codon_.size() + 1);
    std::copy(std::begin(codon_), std::end(codon_), std::begin(genome_) + pos);
  });
}

void markov_cppn::compute_gates_() {

  gates_.clear();
  auto addresses = input_ + output_ + hidden_;
  for (auto pos{std::begin(genome_)}; pos < std::end(genome_) - gene_length_;
       pos++) {
    // find the next codon
    pos = std::search(pos, std::end(genome_) - gene_length_, std::begin(codon_),
                      std::end(codon_));
    if (pos != std::end(genome_) - gene_length_) {
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

      auto [x, y] =
          std::pair{-1 + (*(pos + 13) / 64.0), -1 + (*(pos + 14) / 64.0)};
      auto offset = *(pos + 15) / 128.0;
      // translate logic
      for (auto i : ranges::view::iota(0u, 16)) {
        my_cppn_.input(std::vector{x + offset * i / 4, y + offset * (i % 4)});
        my_cppn_.tick();
        g.logic_.push_back(
            util::Bit(std::get<std::vector<double>>(my_cppn_.output())[0]));
      }

      gates_.push_back(g);
    }
  }
}
