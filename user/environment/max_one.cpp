
#include "max_one.h"

#include <algorithm>

life::population
    max_one::evaluate(life::population pop)
{

  pop.merge(pop.get_as_vector() | ranges::action::transform([this](auto &org) {
              // no inputs
			  org.reset();
              // run single tick
              org.tick();
              // score is number of outputs that evaluate to Bit() == 1
              auto s      = org.output(org_output_ones_tag_);
              auto output = std::get<std::vector<double>>(s);
              org.data.set_value(
                  ones_tag_,
                  static_cast<double>(ranges::count_if(
                      output, [](auto i) { return util::Bit(i); })));
              return org;
            }));
  return pop;
}
