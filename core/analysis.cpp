
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <range/v3/all.hpp>
#include <regex>
#include <vector>

#include "specs/entity_spec.h"
#include "specs/environment_spec.h"
#include "specs/population_spec.h"

#include "analysis.h"
#include "experiments.h"

namespace ded
{

namespace experiments
{
std::tuple<specs::Trace, std::string, std::vector<Simulation>>
    get_single_trace(std::vector<Simulation> simulations,
                     std::string             trace_name)
{

  std::vector<std::tuple<specs::Trace, std::string, std::vector<Simulation>>>
      trace_paths;

  for (auto sim : simulations)
  {
    for (auto trace : sim.traces)
    {
      auto key = ranges::find_if(trace_paths, [trace](auto t) {
        return std::get<1>(t) == trace.second;
      });
      if (key == ranges::end(trace_paths))
        trace_paths.push_back(
            { std::get<0>(trace), std::get<1>(trace), { sim } });
      else
        std::get<2>(*key).push_back(sim);
    }
  }

  std::regex  trace_regex(R"~((\w+)(~\d+)?)~");
  std::smatch m;
  if (!std::regex_match(trace_name, m, trace_regex))
  {
    std::cout << "error: invalid trace name syntax\n";
    throw language::ParserError{};   // wrong exception
  }

  auto base_trace_name = m[1].str();
  auto trace_paths_back_up = trace_paths;
  trace_paths.erase(
      std::remove_if(std::begin(trace_paths),
                     std::end(trace_paths),
                     [&base_trace_name](auto t) {
                       return std::get<0>(t).signal_.user_name() !=
                              base_trace_name;
                     }),
      std::end(trace_paths));

  if (trace_paths.empty())
  {
    std::cout << "error: trace name " << base_trace_name
              << " not a valid trace\nvalid traces are\n";
    for (auto t : trace_paths_back_up)
      std::cout << std::get<1>(t) << std::get<0>(t).signal_.user_name() << "\n";
    throw language::ParserError{};   // wrong exception
  }

  if (trace_paths.size() > 1 && m[2].str().empty())
  {
    std::cout << "error: trace name " << base_trace_name
              << " is repeated\nspecify trace as follows\n";
    for (auto [i,t]  : ranges::view::enumerate(trace_paths))
      std::cout << std::get<1>(t) << std::get<0>(t).signal_.user_name() << "~"
                << i + 1 << "\n";
    throw language::ParserError{};   // wrong exception
  }

  unsigned int n = m[2].str().empty() ? 0 : std::stoi(m[2].str().substr(1)) - 1;

  if (trace_paths.size() <= n)
  {
    std::cout << "error:  trace name " << base_trace_name
              << " does not match\n";
    throw language::ParserError{};   // wrong exception
  }

  return trace_paths[n];
}

std::pair<std::vector<int>, int>
    find_all_stored_data(
        std::tuple<specs::Trace, std::string, std::vector<Simulation>> t)
{
  auto [trace, trace_path, true_simulations] = t;
  std::vector<int> rep_counts;
  std::vector<int> invocation_counts;
  for (const auto &sim : true_simulations)
  {
    auto bar_code = sim.bar_code();
    for (auto rep : ranges::view::iota(0))
    {
      auto rep_path =
          "data/" + bar_code + "/" + std::to_string(rep) + "/" + trace_path;
      // std::cout << rep_path << "\n";
      if (!std::experimental::filesystem::exists(rep_path))
      {
        rep_counts.push_back(rep);
        break;
      }
      int invocation = trace.frequency_;
      while (std::experimental::filesystem::exists(
          rep_path + trace.signal_.user_name() + "_" +
          std::to_string(invocation) + ".csv"))
        invocation += trace.frequency_;
      invocation_counts.push_back(invocation - trace.frequency_);
      // std::cout << invocation << "\n";
    }
  }

  auto [low_rep, high_rep] = ranges::minmax_element(rep_counts);
  if (*low_rep != *high_rep)
  {
    std::cout << "warning: misalignment of replicates\nas many as " << *high_rep
              << " replicates exist\nminimum of " << *low_rep
              << " will be used\n";
  }
  auto [low, high] = ranges::minmax_element(invocation_counts);
  if (*low != *high)
  {
    std::cout << "warning: misalignment of trace invocation counts\nas many as "
              << *high << " invocations exist\nminimum of " << *low
              << " will be used\n";
  }

  return { ranges::view::iota(trace.frequency_, *low + 1) |
               ranges::view::filter(
                   [f = trace.frequency_](auto i) { return !(i % f); }),
           *low_rep };
}

void
    analyse_all_simulations(std::string file_name, std::string trace_name)
{
  auto simulations = ded::experiments::parse_all_simulations(file_name);
  auto single      = get_single_trace(simulations, trace_name);

  auto [trace, trace_path, true_simulations] = single;
  auto [invokes, max_rep]                    = find_all_stored_data(single);

  std::ofstream analysis_file("analysis.R");
  analysis_file << "#!/usr/bin/env Rscript";
  analysis_file << "\nsource(\"dedli/plot.R\")";
  analysis_file << "\nexps=list("
                << (simulations | ranges::view::transform([](auto sim) {
                      return "\"data/" + sim.bar_code() + "/\"";
                    }) |
                    ranges::view::intersperse(",") | ranges::action::join)
                << ")\n";
  analysis_file << "labels=c("
                << (simulations | ranges::view::transform([](auto sim) {
                      return "\"" + sim.full_label() + "\"";
                    }) |
                    ranges::view::intersperse(",") | ranges::action::join)
                << ")\n";
  analysis_file << "data=un_reported_data(exps,\"/" + trace_path << "\",\""
                << trace.signal_.user_name() << "\",0:" << max_rep - 1
                << ",seq(" << trace.frequency_ << "," << invokes.back() << ","
                << trace.frequency_ << "))";
  analysis_file << "\nall_avg = "
                   "compute_all(avg,exps,labels,data)\npdf(\"result.pdf\")\n";
  analysis_file << "cluster_plots(all_avg,\"avg\",labels,1:"
                << simulations.size() << ",palette(rainbow("
                << simulations.size() + 1 << ")),\"" << file_name
                << " Replicates " << max_rep << "\")\n";
  analysis_file << "final_fitness_plots(all_avg,\"Final avg\",labels)";
  analysis_file
      << "\nfor (i in 1:" << simulations.size()
      << ")\n{\n "
         "plot(cluster_plots(all_avg,\"avg\",labels,i:i,palette(rainbow(2)),"
         "labels[i]))\n}\ndev.off()\n";
}

}   // namespace experiments
}   // namespace ded