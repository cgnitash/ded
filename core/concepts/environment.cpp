
#include "environment.h"

#include <experimental/filesystem>

namespace ded
{

namespace concepts
{

Population
    Environment::evaluate(Population p)
{
  if (!user_specified_name_.empty())
    global_path += user_specified_name_[0] + "_";

  auto class_name = self_->class_name_as_string_();
  global_path += class_name + "/";
  std::experimental::filesystem::create_directory(global_path);

  record_traces(p, traces_.pre_);

  auto p_r = self_->evaluate_(p);

  record_traces(p_r, traces_.post_);

  global_path.pop_back();
  global_path = global_path.substr(0, global_path.find_last_of('/') + 1);

  invocations_++;

  return p_r;
}

}   // namespace concepts
}   // namespace ded
