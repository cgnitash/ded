
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <vector>

#include "specs/process_spec.h"
#include "specs/substrate_spec.h"
#include "specs/population_spec.h"

#include "analysis.h"
#include "experiments.h"
#include "utilities/utilities.h"

namespace ded
{

namespace experiments
{
std::tuple<specs::Trace, std::string, std::vector<Simulation>>
    getSingleTrace(std::vector<Simulation> simulations,
                     std::string             trace_name)
{

  std::vector<std::tuple<specs::Trace, std::string, std::vector<Simulation>>>
      trace_paths;

  for (auto sim : simulations)
  {
    for (auto trace : sim.traces)
    {
      auto key = rs::find_if(trace_paths, [trace](auto t) {
        return std::get<1>(t) == trace.second;
      });
      if (key == rs::end(trace_paths))
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
                       return std::get<0>(t).signal_.userName() !=
                              base_trace_name;
                     }),
      std::end(trace_paths));

  if (trace_paths.empty())
  {
    std::cout << "error: trace name " << base_trace_name
              << " not a valid trace\nvalid traces are\n";
    for (auto t : trace_paths_back_up)
      std::cout << std::get<1>(t) << std::get<0>(t).signal_.userName() << "\n";
    throw language::ParserError{};   // wrong exception
  }

  if (trace_paths.size() > 1 && m[2].str().empty())
  {
    std::cout << "error: trace name " << base_trace_name
              << " is repeated\nspecify trace as follows\n";
    for (auto [i,t]  : rv::enumerate(trace_paths))
      std::cout << std::get<1>(t) << std::get<0>(t).signal_.userName() << "~"
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
    findAllStoredData(
        std::tuple<specs::Trace, std::string, std::vector<Simulation>> t)
{
  auto [trace, trace_path, true_simulations] = t;
  std::vector<int> rep_counts;
  std::vector<int> invocation_counts;
  for (const auto &sim : true_simulations)
  {
    auto barCode = sim.barCode();
    auto exp_path = "data/" + barCode + "/";
    if (!std::experimental::filesystem::exists(exp_path))
    {
      std::cout << "Error: cannot analyse this experiment: some simulations "
                   "have not been simulated\n";
      throw language::ParserError{};
    }

    for (auto rep : rv::iota(0))
    {
      auto rep_path =
          exp_path + std::to_string(rep) + "/" + trace_path;
      if (!std::experimental::filesystem::exists(rep_path))
      {
        rep_counts.push_back(rep);
        break;
      }
      int invocation = trace.frequency_;
      while (std::experimental::filesystem::exists(
          rep_path + trace.signal_.userName() + "_" +
          std::to_string(invocation) + ".csv"))
        invocation += trace.frequency_;
      invocation_counts.push_back(invocation - trace.frequency_);
      // std::cout << invocation << "\n";
    }
  }

  auto [low_rep, high_rep] = rs::minmax_element(rep_counts);
  if (*low_rep != *high_rep)
  {
    std::cout << "warning: misalignment of replicates\nas many as " << *high_rep
              << " replicates exist\nminimum of " << *low_rep
              << " will be used\n";
  }
  auto [low, high] = rs::minmax_element(invocation_counts);
  if (*low != *high)
  {
    std::cout << "warning: misalignment of trace invocation counts\nas many as "
              << *high << " invocations exist\nminimum of " << *low
              << " will be used\n";
  }

  return { rv::iota(trace.frequency_, *low + 1) |
               rv::filter(
                   [f = trace.frequency_](auto i) { return !(i % f); }),
           *low_rep };
}

void
    analyseAllSimulations(std::string file_name, std::string trace_name)
{
  auto simulations = ded::experiments::parseAllSimulations(file_name);
  auto single      = getSingleTrace(simulations, trace_name);

  auto [trace, trace_path, true_simulations] = single;
  auto [invokes, max_rep]                    = findAllStoredData(single);

  std::ofstream analysis_file("analysis.R");
  analysis_file << "#!/usr/bin/env Rscript";
  analysis_file << "\nsource(\"dedli/plot.R\")";
  analysis_file << "\nexps=list("
                << (simulations | rv::transform([](auto sim) {
                      return "\"data/" + sim.barCode() + "/\"";
                    }) |
                    rv::intersperse(",") | ra::join)
                << ")\n";
  analysis_file << "labels=c("
                << (simulations | rv::transform([](auto sim) {
                      return "\"" + sim.fullLabel() + "\"";
                    }) |
                    rv::intersperse(",") | ra::join)
                << ")\n";
  analysis_file << "data=un_reported_data(exps,\"/" + trace_path << "\",\""
                << trace.signal_.userName() << "\",0:" << max_rep - 1
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
