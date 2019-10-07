
#pragma once

#include "../../../components.h"

#include <algorithm>

class fork_join {

  std::string info_tag_;
  std::vector<ded::concepts::Process> envs_;
  std::vector<ded::specs::ProcessSpec> env_specs_;

public:
  fork_join() { configure(publishConfiguration()); }

  ded::specs::ProcessSpec publishConfiguration()
  {
    ded::specs::ProcessSpec es;

    es.bindPostTag("info", "double");

	es.bindProcessVector("envs", env_specs_);
    es.bindProcessVectorPostConstraints("envs", { { "info", "double" } });

    return es;
  }

  void configure(ded::specs::ProcessSpec es)
  {
  es.configureProcessVector("envs", env_specs_);
  envs_.clear();
  for (auto &env_spec : env_specs_)
  {
	envs_.push_back(ded::makeProcess(env_spec));
  }

    es.configurePostTag("info", info_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
