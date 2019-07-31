
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
void
    analyse_all_simulations(std::string file_name, std::string trace_name)
{

  std::vector<
      std::pair<std::pair<specs::Trace, std::string>, std::vector<Simulation>>>
      trace_paths;

  auto simulations = ded::experiments::parse_all_simulations(file_name);
  for (auto sim : simulations)
  {
    for (auto trace : sim.traces)
    {
      auto key = ranges::find_if(trace_paths, [trace](auto t) {
        return t.first.second == trace.second;
      });
      if (key == ranges::end(trace_paths))
        trace_paths.push_back({ trace, { sim } });
      else
        key->second.push_back(sim);
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
  trace_paths.erase(
      std::remove_if(std::begin(trace_paths),
                     std::end(trace_paths),
                     [&base_trace_name](auto trace) {
                       return trace.first.first.signal_.user_name() !=
                              base_trace_name;
                     }),
      std::end(trace_paths));

  if (trace_paths.empty())
  {
    std::cout << "error: trace name " << base_trace_name
              << " not a valid trace\nvalid traces are\n";
    for (auto t : trace_paths)
      std::cout << t.first.second << "\n";
    throw language::ParserError{};   // wrong exception
  }

  unsigned int n = m[2].str().empty() ? 0 : std::stoi(m[2].str()) - 1;

  if (trace_paths.size() <= n)
  {
    std::cout << "error:  trace name " << base_trace_name
              << " does not match\n";
    throw language::ParserError{};   // wrong exception
  }

  auto [trace_path, true_simulations] = trace_paths[n];
  // auto &trace_path = trace_path_simulations.first;
  // auto &true_simulations = trace_path_simulations.second;

  // auto trace      = *(simulations.begin()->traces.begin());
  // auto true_trace_name = trace.second + trace.first.signal_.user_name() +
  // "_";
  std::vector<int> rep_counts;
  std::vector<int> invocation_counts;
  for (const auto &sim : true_simulations)
  {
    auto bar_code = sim.bar_code();
    for (auto rep : ranges::view::iota(0))
    {
      auto rep_path = "data/" + bar_code + "/" + std::to_string(rep) + "/" +
                      trace_path.second;
      //std::cout << rep_path << "\n";
      if (!std::experimental::filesystem::exists(rep_path))
      {
        rep_counts.push_back(rep);
        break;
      }
      int invocation = trace_path.first.frequency_;
      while (std::experimental::filesystem::exists(
          rep_path + trace_path.first.signal_.user_name() + "_" +
          std::to_string(invocation) + ".csv"))
        invocation += trace_path.first.frequency_;
      invocation_counts.push_back(invocation - trace_path.first.frequency_);
      //std::cout << invocation << "\n";
    }
  }

  auto [low_rep, high_rep] = ranges::minmax_element(rep_counts);
  if (*low_rep != *high_rep)
  {
    std::cout << "warning: misalignment of replicates\nas many as " << *high_rep
              << " replicates exist\nminimum of " << *low_rep
              << " will be used\n";
    return;
  }
  auto [low, high] = ranges::minmax_element(invocation_counts);
  if (*low != *high)
  {
    std::cout << "warning: misalignment of trace invocation counts\nas many as "
              << *high << " invocations exist\nminimum of " << *low
              << " will be used\n";
    return;
  }

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
  analysis_file << "data=un_reported_data(exps,\"/" + trace_path.second
                << "\",\"" << trace_path.first.signal_.user_name()
                << "\",0:" << *low_rep - 1 << ",seq("
                << trace_path.first.frequency_ << "," << *low << ","
                << trace_path.first.frequency_ << "))";
  analysis_file << "\nall_avg = "
                   "compute_all(avg,exps,labels,data)\npdf(\"result.pdf\")\n";
  analysis_file << "cluster_plots(all_avg,\"avg\",labels,1:"
                << simulations.size() << ",palette(rainbow("
                << simulations.size() + 1 << ")),\"" << file_name
                << " Replicates " << *low_rep << "\")\n";
  analysis_file << "final_fitness_plots(all_avg,\"Final avg\",labels)";
  analysis_file
      << "\nfor (i in 1:" << simulations.size()
      << ")\n{\n "
         "plot(cluster_plots(all_avg,\"avg\",labels,i:i,palette(rainbow(2)),"
         "labels[i]))\n}\ndev.off()\n";
}

}   // namespace experiments
}   // namespace ded
