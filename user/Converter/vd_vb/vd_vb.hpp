
#pragma once

#include <string>
#include <vector>

#include "../../../components.hpp"
#include "../../../core/utilities/utilities.hpp"

class vd_vb
{

public:
  vd_vb()
  {
    configure(publishConfiguration());
  }

  ded::specs::ConverterSpec
      publishConfiguration()
  {
    ded::specs::ConverterSpec es;

    es.bindFrom("<double>");
    es.bindTo( "<int>");


    return es;
  }

  void
      configure(ded::specs::ConverterSpec )
  {

  }

  ded::concepts::Signal
      convert(ded::concepts::Signal s)
  {
    auto v = std::any_cast<std::vector<double>>(s);
    return v | rv::transform(ded::utilities::Bit) | rs::to<std::vector<int>>;
  }
};
