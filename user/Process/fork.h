
#pragma once

#include "../../components.h"

#include <algorithm>

class fork {

  std::string info_tag_;// = "double";
  ded::specs::ProcessSpec one_{"null_process"};
  ded::specs::ProcessSpec two_{"null_process"};
  ded::concepts::Process one = ded::makeProcess(one_);
  ded::concepts::Process two = ded::makeProcess(two_);

public:
  fork() { configure(publishConfiguration()); }

  ded::specs::ProcessSpec publishConfiguration()
  {
    ded::specs::ProcessSpec es;

    es.bindPostTag("info", "double");

	es.bindProcess("one",one_);
    es.bindProcessPostConstraints("one", { { "info", "double" } });

	es.bindProcess("two",two_);
    es.bindProcessPostConstraints("two", { { "info", "double" } });

    return es;
  }

  void configure(ded::specs::ProcessSpec es)
  {
    es.configureProcess("one",one_);
	one = ded::makeProcess(one_);

    es.configureProcess("two",two_);
	two = ded::makeProcess(two_);

    es.configurePostTag("info", info_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
