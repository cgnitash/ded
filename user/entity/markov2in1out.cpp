
#include"markov2in1out.h"
#include"../../core/utilities.h"

#include <vector>
#include <algorithm>
#include <regex>

void markov2in1out::mutate() {

  life::all_mutations(genome_);
  gates_valid_ = false;
  gates_.clear();
  buffer_ = std::vector<double>(input_ + output_ + hidden_, 0);
}

void markov2in1out::input(life::signal v) {
	for(auto i=0;i<input_;i++)
		buffer_[i] = util::bit(v[i]);
}


life::signal markov2in1out::output() {
  life::signal v(output_);
  for (auto i = 0; i < output_; i++)
    v[i] = util::bit(buffer_[i + input_]);
  return v;
}

void markov2in1out::tick() {

  if (!gates_valid_)
    compute_gates_();

  std::vector<double> out_buffer(buffer_.size(), 0);

  for (auto &g : gates_)
    out_buffer[g.out_] +=
        g.logic_[util::bit(buffer_[g.in_1_]) * 2 + util::bit(buffer_[g.in_2_])];

  buffer_ = out_buffer;
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
  for (auto pos = std::begin(genome_);pos<std::end(genome_)-10;pos++) {
    pos = std::find(pos, std::end(genome_) - 10, 2);
    if (pos == std::end(genome_)-10 || *(pos+1) != 8 ) {
    continue;	
    }
    gate g{*(pos + 2) % addresses,
           *(pos + 3) % addresses,
           *(pos + 4) % addresses,
           {static_cast<double>(*(pos + 5) % 2),
            static_cast<double>(*(pos + 6) % 2),
            static_cast<double>(*(pos + 7) % 2),
            static_cast<double>(*(pos + 8) % 2)}};
    gates_.push_back(g);
  }
  gates_valid_ = true;
//` std::cout << gates_.size() << " ";
}
