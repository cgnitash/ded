
#pragma once

#include <string>
#include <vector>

#include "../../../components.hpp"
#include "../../../core/utilities/utilities.hpp"

class add
{

  double by_ = 0;

public:
  void
      configuration(ded::specs::ConverterSpec &es)
  {
    es.parameter("by", by_);
    es.bindFrom("<double,_>");
    es.bindTo("<double,_>");
  }

  ded::concepts::Signal
      convert(ded::concepts::Signal s)
  {
    auto v = std::any_cast<std::vector<double>>(s);
    return v | rv::transform([this](auto d) { return d + by_; }) |
           rs::to<std::vector<double>>;
  }
};
