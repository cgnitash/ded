
#pragma once

#include "../../../components.hpp"

#include <string>

class elite_replacement
{

public:
  void
      configuration(ded::specs::ProcessSpec &spec)
  {
    spec.parameter("strength",
                   strength_,
                   { { [](double s) { return 0.0 < s && s < 1.0; },
                       "strength must be in the range (0.0, 1.0)" } });

    spec.preTag("value", "double");
  }

  ded::concepts::Population evaluate(ded::concepts::Population);

private:
  double strength_ = 0.1;
  int    invoke_   = 0;
};

