
#include "max_one.h"
#include "../../core/utilities.h"
#include <algorithm>

life::population max_one::evaluate(life::population pop) {

  pop.merge(pop.get_as_vector() | ranges::action::transform([](auto &org) {
              // no inputs
              // run single tick
              org.tick();
              // score is number of outputs that evaluate to Bit() == 1
              auto s = org.output();
              if (auto vp = std::get_if<std::vector<double>>(&s)) {
                auto output = *vp;
                org.data["score"] = ranges::count_if(
                    output, [](auto i) { return util::Bit(i); });
                return org;
              } else {
                std::cout << "Error: entity-max-one cannot handle this "
                             "payload type \n";
                exit(1);
              }
            }));
  return pop;
}

