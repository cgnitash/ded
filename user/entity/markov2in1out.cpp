
#include"markov2in1out.h"

#include <vector>
#include <algorithm>
#include <regex>

void markov2in1out::mutate() {
  auto copy_prob = rand() % 1000;
  if (!copy_prob) {
    auto copy_from_pos = rand() % (genome_.length() - 40);
    auto copy_to_pos = rand() % genome_.length();
    genome_ = genome_.insert(copy_to_pos, genome_, copy_from_pos, 40);
  }

  auto point_mut = rand() % 3;
  for (auto i = 0; i < point_mut; i++)
    genome_[rand() % genome_.length()] = static_cast<char>(rand());

  auto point_ins = rand() % 3;
  for (auto i = 0; i < point_ins; i++)
    genome_.insert(std::begin(genome_) + rand() % genome_.length(),
                   static_cast<char>(rand()));

  auto point_del = rand() % 3;
  for (auto i = 0; i < point_del; i++)
    genome_.erase(std::begin(genome_) + rand() % genome_.length());

  gates_valid_ = false;
  gates_.clear();
  buffer_ = std::vector<long>(input_ + output_ + hidden_, 0);
}

void markov2in1out::input(life::signal v) {
	for(auto i=0;i<input_;i++)
		buffer_[i] = v[i];
}

life::signal markov2in1out::output() {
  life::signal v(output_);
  for (auto i = 0; i < output_; i++)
    v[i] = buffer_[i + input_];
  return v;
}

void markov2in1out::tick() {

  if (!gates_valid_)
    compute_gates_();

  std::vector<long> out_buffer(buffer_.size(), 0);

  for (auto &g : gates_)
    out_buffer[g.out_] = g.logic_[g.in_1_ * 2 + g.in_2_];

  buffer_ = out_buffer;
}

void markov2in1out::compute_gates_() {

	gates_.clear();
  auto addresses = input_ + output_ + hidden_;
  std::regex r(R"(cd(.{7}))");
  for (std::sregex_iterator end, i(std::begin(genome_), std::end(genome_), r);
       i != end; ++i) {
    std::smatch m = *i;
    auto c = m[1].str();
    gate g{c[0] % addresses,
           c[1] % addresses,
           c[2] % addresses,
           {c[3] % 2, c[4] % 2, c[5] % 2, c[6] % 2}};

    gates_.push_back(g);
  }
  gates_valid_ = true;
}
