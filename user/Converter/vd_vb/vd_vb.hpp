
#pragma once

#include <string>
#include <vector>

#include "../../../components.hpp"
#include "../../../core/utilities/utilities.hpp"

class vd_vb
{

public:
  void
      configuration(ded::specs::ConverterSpec &es)
  {
    es.bindTo("<double,_>");
    es.bindFrom("<bool,_>");
  }

  ded::concepts::Signal
      convert(ded::concepts::Signal s)
  {
    auto v = std::any_cast<std::vector<double>>(s);
    return v | rv::transform(ded::utilities::Bit) | rs::to<std::vector<bool>>;
  }
};
