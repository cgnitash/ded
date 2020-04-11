
#pragma once

#include "../../../components.hpp"

class sequence
{

public:

  void
      configuration(ded::specs::ProcessSpec &es)
  {
    es.nestedProcess("first", first_);
    es.nestedProcess("second", second_);
    first  = ded::makeProcess(first_);
    second = ded::makeProcess(second_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);

private:
  ded::specs::ProcessSpec first_{ "null_process" };
  ded::specs::ProcessSpec second_{ "null_process" };
  ded::concepts::Process  first  = ded::makeProcess(first_);
  ded::concepts::Process  second = ded::makeProcess(second_);
};
