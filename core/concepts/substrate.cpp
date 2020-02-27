
#include "substrate.hpp"
#include "../../components.hpp"
#include "converter.hpp"

namespace ded
{
namespace concepts
{

void
    Substrate::input(specs::ConversionSequence conversion_sequence, Signal s)
{
  for (auto conversion : conversion_sequence)
  {
    auto sp = s;
    for(auto &c : conversion.sequence_)
	{
      sp     = c(sp);
    }
    self_->input_(conversion.sink_, sp);
  }
}

Signal
    Substrate::output(specs::ConversionSequence sequence)
{
  Signal sp;
  for (auto conversion : sequence)
  {
    sp = self_->output_(conversion.source_);
    for(auto &c : conversion.sequence_)
    {
	  // actually have to blend sp somehow
      sp     = c(sp);
    }
  }
  return sp;
}

}   // namespace concepts
}   // namespace ded
