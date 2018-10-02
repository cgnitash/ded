
#include"markov_brain.h"

#include <vector>
#include <algorithm>
#include <regex>

void markov_brain::mutate() {
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

void markov_brain::input(life::signal v) {
	for(auto i=0;i<input_;i++)
		buffer_[i] = v[i];
}

life::signal markov_brain::output() {
  life::signal v(output_);
  for (auto i = 0; i < output_; i++)
    v[i] = buffer_[i + input_];
  return v;
}

void markov_brain::tick() {

  if (!gates_valid_)
    compute_gates_();

  std::vector<long> out_buffer(buffer_.size(), 0);

  for (auto &g : gates_) {
    auto in = 0;
    for (auto &i : g.ins_)
      in += in * 2 + buffer_[i];
    auto out = g.logic_[in];
    for (auto &i : g.outs_) {
      out_buffer[i] |= out % 2;
      out /= 2;
    }
  }
  buffer_ = out_buffer;
}

void markov_brain::compute_gates_() {

  auto addresses = input_ + output_ + hidden_;
  std::regex r(R"(ab(.)(....)(.)(....)(.){16})");
  for (std::sregex_iterator end, i(std::begin(genome_), std::end(genome_), r);
       i != end; ++i) {
    std::smatch m = *i;
    gate g;
    auto in_num = std::stol(m[1]) % addresses;
    auto ins_val = m[2].str();
    for (int i = 0; i < in_num; i++)
      g.ins_.push_back(ins_val[i] % addresses);
    auto out_num= std::stol(m[3]) % addresses;
    auto out_val  = m[4].str();
    for (int i = 0; i < out_num; i++)
      g.outs_.push_back(out_val[i] % addresses);
    auto logic = m[5].str();
    for (auto& c:logic) 
      g.logic_.push_back(c % 16);
	gates_.push_back(g);
  }
  gates_valid_ = true;
}
