#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

class max_one
{

  long length_ = 10;

  ded::specs::ConversionSignatureSequence output_ones;

public:
  void
      configuration(ded::specs::ProcessSpec &spec)
  {
    spec.parameter("length", length_);
    spec.postTag("ones", "double");
    spec.output("ons", "<double,length>", output_ones);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
