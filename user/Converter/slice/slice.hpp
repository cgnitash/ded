
#pragma once

#include <string>
#include <vector>

#include "../../../components.hpp"
#include "../../../core/utilities/utilities.hpp"

class slice
{

  long from_  = 0;
  long to_    = 0;
  long every_ = 1;

  std::string vector_template_type_ = "bool";

public:
  void
      configuration(ded::specs::ConverterSpec &es)
  {
    es.parameter("from", from_);
    es.parameter("to", to_);
    es.parameter(
        "every",
        every_,
        { { [](long every) { return every > 0; }, "every must be positive" } });
    es.bindFrom("<_,_>");
    es.bindTo("<_,_>");

    es.parameter("vtt", vector_template_type_);
  }

  ded::concepts::Signal
      convert(ded::concepts::Signal s)
  {
    // doesn't matter, will be generated
    return s;
  }
};
