
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

class max_one {

public:
  max_one() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() { return life::configuration(); }

  void configure(life::configuration) {}

   life::population evaluate(life::population);
};

