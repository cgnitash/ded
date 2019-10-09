
#include <experimental/filesystem>
#include <string>
#include <fstream>
#include <iostream>

#include "process.hpp"

namespace ded
{
namespace concepts
{

Population
    Process::evaluate(Population p)
{
  if (!user_specified_name_.empty())
    GLOBAL_PATH += user_specified_name_[0] + "_";

  auto class_name = self_->classNameAsString_();
  GLOBAL_PATH += class_name + "/";
  std::experimental::filesystem::create_directory(GLOBAL_PATH);

  recordTraces(p, traces_.pre_);

  auto p_r = self_->evaluate_(p);

  recordTraces(p_r, traces_.post_);

  GLOBAL_PATH.pop_back();
  GLOBAL_PATH = GLOBAL_PATH.substr(0, GLOBAL_PATH.find_last_of('/') + 1);

  invocations_++;

  return p_r;
}

void
    Process::configure(specs::ProcessSpec es)
{
  traces_ = es.traces();

  user_specified_name_ = { es.getUserSpecifiedName() };

  self_->configure_(es);
}
void
    Process::recordTraces(const Population &pop, std::vector<specs::Trace> ts)
{

  for (auto n : ts)
    if (invocations_ && !(invocations_ % n.frequency_))
    {
      specs::SignalSpec s_spec{ n.signal_ };
      std::ofstream pop_stats_file{ ded::GLOBAL_PATH + s_spec.userName() + "_" +
                                    std::to_string(invocations_) + ".csv" };
      pop_stats_file << "id," << s_spec.userName() << "\n";
      for (const auto &org : pop.getAsVector())
        pop_stats_file << org.getID() << ","
                       << std::get<double>(
                              org.data.getValue(s_spec.identifier()))
                       << std::endl;
    }
}
}   // namespace concepts
}   // namespace ded
