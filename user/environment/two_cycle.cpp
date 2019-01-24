
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

life::population
    two_cycle::evaluate(life::population pop)
{

  life::global_path += "two_cycle/";

  if (std::experimental::filesystem::exists(life::global_path))
  {
    std::cout
        << "error: directory \"" << life::global_path
        << "\" already contains data. This will be overwritten. aborting..."
        << std::endl;
    exit(1);
  }
  std::experimental::filesystem::create_directory(life::global_path);

  auto optimiser = life::make_environment(sel_);

  auto fit_fun = life::make_environment(fitn_);

  for (auto i : ranges::view::iota(0, count_))
  {

    pop = fit_fun.evaluate(pop);

    if (pop_av_freq_ && !(i % pop_av_freq_))
    {
      std::ofstream pop_stats_file{ life::global_path + "score_" +
                                    std::to_string(i) + ".csv" };
      pop_stats_file << "id,score\n";
      for (auto &org : pop.get_as_vector())
        pop_stats_file << org.get_id() << ","
                       << std::get<double>(org.data.get_value(score_tag_))
                       << std::endl;
    }

    if (cout_freq_ && !(i % cout_freq_))
    {
      auto       v = pop.get_as_vector();
      const auto scores =
          v | ranges::view::transform([this](const auto &org) {
            return std::get<double>(org.data.get_value(score_tag_));
          });
      std::cout << "update:" << std::setw(6) << i << "   "
                << "max:" << std::setw(6) << *ranges::max_element(scores)
                << "   "
                << "ave:" << std::setw(6)
                << ranges::accumulate(scores, 0.0) / pop.size() << std::endl;
    }

    if (snap_freq_ && !(i % snap_freq_)) { pop.snapshot(i); }

    pop = optimiser.evaluate(pop);

    // this should not be necessary to say
    pop.prune_lineage(i);
  }
  // this should not be necessary to say
  pop.flush_unpruned();

  // yuck
  life::global_path =
      life::global_path.substr(0, life::global_path.length() - 10);

  return pop;
}
