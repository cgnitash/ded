
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

  size_t  size_ = 10;
  double eval(life::entity &);

public:
  flip_bits() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration c;
    c["size"] = size_;
    return c;
  }

  void configure(life::configuration c) {
	size_ = c["size"];
  }

  void evaluate(std::vector<life::entity> &);
};

