
#pragma once

#include <string>
#include <vector>

#include "../../../components.hpp"
#include "../../../core/utilities/utilities.hpp"

class vb_vd
{

public:
  vb_vd()
  {
    configure(publishConfiguration());
  }

  ded::specs::ConverterSpec
      publishConfiguration()
  {
    ded::specs::ConverterSpec es;

    es.bindTo("<double,_>");
    es.bindFrom( "<bool,_>");


    return es;
  }

  void
      configure(ded::specs::ConverterSpec )
  {

  }

  ded::concepts::Signal
      convert(ded::concepts::Signal s)
  {
    auto v = std::any_cast<std::vector<bool>>(s);
    return v |  rs::to<std::vector<double>>;
  }
};
