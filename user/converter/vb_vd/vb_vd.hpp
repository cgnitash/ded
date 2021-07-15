
#pragma once

#include <string>
#include <vector>

#include "../../../components.hpp"
#include "../../../core/utilities/utilities.hpp"

class vb_vd
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
    auto v = std::any_cast<std::vector<bool>>(s);
    return v | rs::to<std::vector<double>>;
  }
};
