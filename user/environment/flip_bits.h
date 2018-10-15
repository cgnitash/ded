
#include"../../components.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <regex>
#include <set>
#include <initializer_list>

class flip_bits {

	long size_ = 10;
	long reps_ = 10;
public:
  flip_bits() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration c;
	c["size"] = size_;
	c["reps"] = reps_;
    return c;
  }

  void configure(life::configuration c) {
	size_ = c["size"];
	reps_ = c["reps"];
  }

  life::eval_results evaluate(const std::vector<life::entity> &);

  double eval(life::entity);
};

