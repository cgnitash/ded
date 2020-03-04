
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "../../components.hpp"
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

  p_r = applyTagConversions(p_r);

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

  auto tag_conversions = es.getTagConversions();
  for (auto &conversion : tag_conversions)
  {
    specs::ConversionSignatureSequence_ css;
    css.source_ = conversion.source_;
    css.sink_   = conversion.sink_;
    for (auto s : conversion.specs_)
    {
      auto c = makeConverter(s);
      css.sequence_.push_back(c.getConversionFunction());
    }
    tag_conversions_.push_back(css);
  }

  self_->configure_(es);
}

void
    Process::recordTraces(const Population &pop, std::vector<specs::Trace> ts)
{

  for (auto n : ts)
    if (invocations_ && !(invocations_ % n.frequency_))
    {
      std::ofstream pop_stats_file{ ded::GLOBAL_PATH + n.name_ + "_" +
                                    std::to_string(invocations_) + ".csv" };
      pop_stats_file << "id," << n.name_ << "\n";
      for (const auto &org : pop.getAsVector())
        pop_stats_file << org.getID() << ","
                       << std::any_cast<double>(org.data.getValue(n.name_))
                       << std::endl;
    }
}

concepts::Population
    Process::applyTagConversions(concepts::Population pop)
{

  auto vec = pop.getAsVector();

  auto convert_org = [this](auto org) {
    for (auto cs : tag_conversions_)
    {
      auto sig = org.data.getValue(cs.source_);
      for (auto c : cs.sequence_)
        sig = c(sig);
      org.data.setValue(cs.sink_, sig);
      org.data.clear(cs.source_);
    }
    return org;
  };

  ra::transform(vec, convert_org);
  pop.merge(vec);
  return pop;
}

}   // namespace concepts
}   // namespace ded
