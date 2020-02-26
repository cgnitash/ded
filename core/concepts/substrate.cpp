
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
  for (auto seq : conversion_sequence)
  {
    auto sp = s;
    //for (auto i = 1u; i < seq.sequence_.size() - 1; i++)
    for(auto &c : seq.sequence_)
	{
      //auto c = makeConverter(ALL_CONVERTER_SPECS[seq.sequence_[i]]);
      sp     = c(sp);
    }
    self_->input_(seq.sink_, sp);
  }
}

Signal
    Substrate::output(specs::ConversionSequence sequence)
{
  Signal sp;
  for (auto seq : sequence)
  {
    sp = self_->output_(seq.sink_);
    //for (auto i = 1u; i < seq.size() - 1; i++)
    for(auto &c : seq.sequence_)
    {
      //auto c = makeConverter(ALL_CONVERTER_SPECS[seq[i]]);

	  // actually have to blend sp somehow
      sp     = c(sp);
    }
  }
  return sp;
}

}   // namespace concepts
}   // namespace ded
