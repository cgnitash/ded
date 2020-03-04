
#pragma once

#include "../../../components.hpp"

#include <string>

class elite_replacement {

  double      strength_  = 0.1;
  int invoke_ = 0;

public:
  elite_replacement() { configure(publishConfiguration()); }

  ded::specs::ProcessSpec publishConfiguration()
  {
    ded::specs::ProcessSpec es;

    es.bindParameter("strength",
                      strength_,
                      { { [](double s) { return 0.0 < s && s < 1.0; },
                          "strength must be in the range (0.0, 1.0)" } });

    es.bindPreTag("value", "double");

    return es;
  }

  void configure(ded::specs::ProcessSpec es)
  {
    es.configureParameter("strength",strength_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};

