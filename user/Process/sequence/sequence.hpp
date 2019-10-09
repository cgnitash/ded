
#pragma once

#include "../../../components.hpp"

class sequence {

  ded::specs::ProcessSpec first_{"null_process"};
  ded::specs::ProcessSpec second_{"null_process"};
  ded::concepts::Process first = ded::makeProcess(first_);
  ded::concepts::Process second = ded::makeProcess(second_);

public:
  sequence() { configure(publishConfiguration()); }

  ded::specs::ProcessSpec publishConfiguration()
  {
    ded::specs::ProcessSpec es;

	es.bindProcess("first",first_);

	es.bindProcess("second",second_);

	es.bindTagEquality({"first", "post"}, {"second", "pre"});

    return es;
  }

  void configure(ded::specs::ProcessSpec es)
  {
    es.configureProcess("first",first_);
    es.configureProcess("second",second_);
	first = ded::makeProcess(first_);
	second = ded::makeProcess(second_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
