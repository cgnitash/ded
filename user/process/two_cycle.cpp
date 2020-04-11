
#include "two_cycle.h"

#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

ded::population
    two_cycle::evaluate(ded::population pop)
{

  auto optimiser = ded::makeprocess(sel_);

  auto fit_fun = ded::makeprocess(fitn_);

  for (auto i : rv::iota(0, count_))
  {

    pop = fit_fun.evaluate(pop);

    if (pop_av_freq_ && !(i % pop_av_freq_))
    {
      std::ofstream pop_stats_file{ ded::global_path + "score_" +
                                    std::to_string(i) + ".csv" };
      pop_stats_file << "id,score\n";
      for (auto &org : pop.getAsVector())
        pop_stats_file << org.getID() << ","
                       << std::get<double>(org.data.get_value(score_tag_))
                       << std::endl;
    }

    if (cout_freq_ && !(i % cout_freq_))
    {
      auto       v = pop.getAsVector();
      const auto scores =
          v | rv::transform([this](const auto &org) {
            return std::get<double>(org.data.get_value(score_tag_));
          });
      std::cout << "update:" << std::setw(6) << i << "   "
                << "max:" << std::setw(6) << *rs::max_element(scores)
                << "   "
                << "ave:" << std::setw(6)
                << rs::accumulate(scores, 0.0) / pop.size() << std::endl;
    }

    if (snap_freq_ && !(i % snap_freq_)) { pop.snapshot(i); }

    pop = optimiser.evaluate(pop);

    // this should not be necessary to say
    pop.prune_lineage(i);
  }
  // this should not be necessary to say
  pop.flush_unpruned();

  return pop;
}
