
#pragma once

#include "../../../components.hpp"

#include <algorithm>

class fork_join {

  std::vector<ded::concepts::Process> envs_;
  std::vector<ded::specs::ProcessSpec> env_specs_;

public:
  void configuration(ded::specs::ProcessSpec &ps)
  {
  ps.nestedProcessVector("envs", env_specs_,{}, {{"x", "double"}});
  envs_.clear();
  for (auto &env_spec : env_specs_)
  {
	envs_.push_back(ded::makeProcess(env_spec));
  }

    ps.postTag("info", "double");
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
