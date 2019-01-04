
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

life::population two_cycle::evaluate(life::population pop) {

  life::global_path += "two_cycle/";

  if (std::experimental::filesystem::exists(life::global_path)) {
    std::cout
        << "error: directory \"" << life::global_path
        << "\" already contains data. This will be overwritten. aborting..."
        << std::endl;
    exit(1);
  }
  std::experimental::filesystem::create_directory(life::global_path);

  auto optimiser = life::make_environment(sel_name_);
  optimiser.configure(sel_config_);

  auto fit_fun = life::make_environment(fitn_name_);
  fit_fun.configure(fitn_config_);

  for (auto i : ranges::view::iota(0, count_)) {

    pop = fit_fun.evaluate(pop);

    auto fitnesses = [this](const auto &vec) {
      life::configuration cout_con;

      const auto scores = vec | ranges::view::transform([this](const auto &org) {
                        return double{org.data.get_value(score_tag_)};
                      });

      cout_con["max"] = *ranges::max_element(scores);
      cout_con["avg"] = ranges::accumulate(scores, 0.0) / vec.size();

      return cout_con;
    };

    if (pop_av_freq_ && !(i % pop_av_freq_)) {
      auto cout_con = fitnesses(pop.get_as_vector());
      auto pop_stats_file = util::open_or_append(life::global_path + "pop.csv",
                                                 "avg,max,update\n");
      pop_stats_file << cout_con["avg"] << "," << cout_con["max"] << "," << i
                     << std::endl;
    }

    if (cout_freq_ && !(i % cout_freq_)) {
      auto cout_con = fitnesses(pop.get_as_vector());
      std::cout << "update:" << std::setw(6) << i << "   " << cout_con
                << std::endl;
    }

    if (snap_freq_ && !(i % snap_freq_)) {
      pop.snapshot(i);
	}

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

