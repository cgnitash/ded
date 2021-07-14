#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class logic_9 {

public:

  void configuration(ded::specs::ProcessSpec &spec)
  {

    spec.parameter("bits",bits_);
    spec.parameter("tests",tests_);
    spec.postTag("logic_done","double");

    spec.input("args","<double,bits * 2>",args);
    spec.output("result", "<double,bits>", result);

  }

  ded::concepts::Population evaluate(ded::concepts::Population);

private:
  ded::specs::ConversionSignatureSequence args;
  ded::specs::ConversionSignatureSequence result;

  long   tests_    = 10;
  long   bits_    = 4;
  double eval(ded::concepts::Substrate &);
};
