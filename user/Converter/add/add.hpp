
#pragma once

#include <string>
#include <vector>

#include "../../../components.hpp"
#include "../../../core/utilities/utilities.hpp"

class add
{

	double by = 0;

public:
  add()
  {
    configure(publishConfiguration());
  }

  ded::specs::ConverterSpec
      publishConfiguration()
  {
    ded::specs::ConverterSpec es;

	es.bindParameter("by", by);
    es.bindFrom("<double,_>");
    es.bindTo( "<double,_>");


    return es;
  }

  void
      configure(ded::specs::ConverterSpec es)
  {
	es.configureParameter("by", by);

  }

  ded::concepts::Signal
      convert(ded::concepts::Signal s)
  {
    auto v = std::any_cast<std::vector<double>>(s);
    return v | rv::transform([this](auto d) { return d + by; }) |
           rs::to<std::vector<double>>;
  }
};
