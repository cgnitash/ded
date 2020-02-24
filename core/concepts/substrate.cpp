
#include "substrate.hpp"
#include "../../components.hpp"
#include "converter.hpp"

namespace ded
{
namespace concepts
{

void
    Substrate::input(specs::SignalConversionSequence sequence, Signal s)
{
  for (auto seq : sequence)
  {
    auto sp = s;
    for (auto i = 1u; i < seq.size() - 1; i++)
    {
      auto c = makeConverter(ALL_CONVERTER_SPECS[seq[i]]);
      sp     = c.convert(sp);
    }
    self_->input_(seq.back(), sp);
  }
}

Signal
    Substrate::output(specs::SignalConversionSequence sequence)
{
  Signal sp;
  for (auto seq : sequence)
  {
    sp = self_->output_(seq.front());
    for (auto i = 1u; i < seq.size() - 1; i++)
    {
      auto c = makeConverter(ALL_CONVERTER_SPECS[seq[i]]);

	  // actually have to blend sp somehow
      sp     = c.convert(sp);
    }
  }
  return sp;
}

}   // namespace concepts
}   // namespace ded
