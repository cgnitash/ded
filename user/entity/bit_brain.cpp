
#include "bit_brain.h"

#include <algorithm>
#include <vector>

void bit_brain::tick() {}

void bit_brain::mutate() {
  auto i = rand() % size_;
  genome_[i] = static_cast<char>(rand()); 
}

life::signal bit_brain::output() {
	life::signal v;
	std::transform(std::begin(genome_), std::end(genome_),
	               std::back_inserter(v),
	               [](auto const value) {
		return value %2;
	});
  	return v;
}

void bit_brain::input(life::signal) {}

