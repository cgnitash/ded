
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/contains.hpp>
#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/algorithm/min_element.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/enumerate.hpp>
#include <regex>
#include <vector>

#include "specs/population_spec.hpp"
#include "specs/process_spec.hpp"
#include "specs/substrate_spec.hpp"

#include "analysis.hpp"
#include "experiments.hpp"
#include "utilities/csv/csv.hpp"
#include "utilities/utilities.hpp"

namespace ded
{

namespace experiments
{
std::vector<std::tuple<specs::Trace, std::string, std::vector<Simulation>>>
    getAllTraces(std::vector<Simulation> simulations, std::string trace_name)
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

  auto trace_paths_back_up = trace_paths;
  trace_paths.erase(std::remove_if(std::begin(trace_paths),
                                   std::end(trace_paths),
                                   [&trace_name](auto t) {
                                     return std::get<0>(t).name_ != trace_name;
                                   }),
                    std::end(trace_paths));

  if (trace_paths.empty())
  {
    std::cout << "error: trace name " << trace_name
              << " not a valid trace\nvalid traces are\n";
    for (auto t : trace_paths_back_up)
      std::cout << std::get<1>(t) << std::get<0>(t).name_ << "\n";
    throw language::ParserError{};   // wrong exception
  }

  return trace_paths;
}

double
    averageMean(std::vector<double> const &v)
{
  return rs::accumulate(v, 0.) / v.size();
}

double
    standardDeviation(std::vector<double> const &v)
{
  auto d = [m = averageMean(v)](auto n) { return std::abs(n - m); };
  return std::sqrt(rs::accumulate(v | rv::transform(d), 0.));
}

std::pair<std::vector<std::vector<std::vector<std::string>>>, int>
    findAllStoredData(
        std::tuple<specs::Trace, std::string, std::vector<Simulation>> t)
{
  auto [trace, trace_path, true_simulations] = t;
  std::vector<int>                                   rep_counts;
  std::vector<int>                                   invocation_counts;
  std::vector<std::vector<std::vector<std::string>>> all_files;
  for (const auto &sim : true_simulations)
  {
    auto barCode  = sim.barCode();
    auto exp_path = "data/" + barCode + "/";
    if (!std::experimental::filesystem::exists(exp_path))
    {
      std::cout << "Error: cannot analyse this experiment: some simulations "
                   "have not been simulated\n";
      throw language::ParserError{};
    }
    std::vector<std::vector<std::string>> rep_files;
    for (auto rep : rv::iota(0))
    {
      auto rep_path = exp_path + std::to_string(rep) + "/" + trace_path;
      if (!std::experimental::filesystem::exists(rep_path))
      {
        rep_counts.push_back(rep);
        break;
      }
      std::vector<std::string> files;
      std::vector<std::string> true_files;
      for (auto p :
           std::experimental::filesystem::directory_iterator{ rep_path })
        files.push_back(p.path());
      int  invocation = trace.frequency_;
      auto trace_name = trace.name_;
      while (true)
      {
        auto patt = std::string{ ".*" } + trace_name + "_" +
                    std::to_string(invocation) + "_\\d+.csv";
        auto r  = std::regex{ patt };
        auto fi = rs::find_if(
            files, [&](auto const &f) { return std::regex_match(f, r); });
        if (fi == rs::end(files))
          break;
        invocation += trace.frequency_;
        true_files.push_back(*fi);
      }
      invocation_counts.push_back(invocation - trace.frequency_);
      rep_files.push_back(true_files);
    }
    all_files.push_back(rep_files);
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

  return { all_files, *low_rep };
}

void
    generateAllStoredData(
        std::tuple<specs::Trace, std::string, std::vector<Simulation>> t,
        int                                                            index)
{
  auto [invokes, max_rep]                    = findAllStoredData(t);
  auto [trace, trace_path, true_simulations] = t;

  for (const auto &[i, sim] : rv::enumerate(true_simulations))
  {
    auto barCode  = sim.barCode();
    auto exp_path = "data/" + barCode + "/";

    auto alldata = std::ofstream{ exp_path + trace.name_ + "_" +
                                  std::to_string(index) + ".csv" };
    alldata << "update,min,max,avg,minsd,maxsd,avgsd\n";

    for (auto invocation : rv::iota(0u, invokes[0][0].size()))
    {
      std::vector<double> mins, maxs, avgs;
      for (auto rep : rv::iota(0, max_rep))
      {
        auto filepath = invokes[i][rep][invocation];
        auto cf       = ded::utilities::csv::CSV{ filepath };
        auto scores   = cf.single_column(trace.name_);
        auto scd = scores | rv::transform([](auto s) { return std::stod(s); }) |
                   rs::to<std::vector<double>>;
        mins.push_back(*rs::min_element(scd));
        maxs.push_back(*rs::max_element(scd));
        avgs.push_back(rs::accumulate(scd, 0.) / scd.size());
      }
      alldata << invocation << "," << averageMean(mins) << ","
              << averageMean(maxs) << "," << averageMean(avgs) << ","
              << standardDeviation(mins) << "," << standardDeviation(maxs)
              << "," << standardDeviation(avgs) << "\n";
    }
  }
}

void
    analyseAllSimulations(std::string file_name, std::string trace_query)
{
  auto simulations = ded::experiments::parseAllSimulations(file_name);

  auto original_trace_query = trace_query;
  if (trace_query.find('~') == std::string::npos)
    trace_query += "~1";

  auto split_point = trace_query.find('~');
  auto trace_name  = trace_query.substr(0, split_point);
  auto ind_strings = trace_query.substr(split_point + 1);
  auto inds = ind_strings | rv::split(',') | rs::to<std::vector<std::string>>;

  auto traces = getAllTraces(simulations, trace_name);
  for (auto [i, trace] : rv::enumerate(traces))
    generateAllStoredData(trace, i + 1);

  if (original_trace_query.find('~') == std::string::npos and traces.size() > 1)
    std::cout << "warning: up to " << traces.size()
              << " subscripts exist, only 1 will be used\n";

  std::ofstream analysis_file("analysis.R");
  analysis_file << "require(\"ggplot2\")\n";
  analysis_file << "\nexps=list("
                << (simulations | rv::transform([](auto sim) {
                      return "\"data/" + sim.barCode() + "/\"";
                    }) |
                    rv::intersperse(",") | ra::join)
                << ")\n";
  analysis_file << "exp_labels=c("
                << (simulations | rv::transform([](auto sim) {
                      return "\"" + sim.fullLabel() + "\"";
                    }) |
                    rv::intersperse(",") | ra::join)
                << ")\n";
  analysis_file << "gsdf = function(sim) {\ndata.frame(rbind(";
  for (auto i_ind = 0u; i_ind < inds.size(); i_ind++)
  {
    auto ind = inds[i_ind];
    if (not std::regex_match(ind, std::regex{ R"~(^\d+$)~" }))
    {
      std::cout << "error:  trace subscript " << ind << " is not an integer\n";
      throw language::ParserError{};   // wrong exception
    }
    if (auto i = std::stoul(ind); i < 1 or i > traces.size())
    {
      std::cout << "error:  trace subscript " << ind
                << " is not valid; must be in the range 1.." << traces.size()
                << "\n";
      throw language::ParserError{};   // wrong exception
    }
    analysis_file << "read.csv(paste(exps[sim],\"" << trace_name << "_" << ind
                  << ".csv\",sep = \"\"))";
    if (i_ind != inds.size() - 1)
      analysis_file << ",\n";
  }
  analysis_file << "))\n}\npdf(\"result.pdf\")\nggplot() +\n";
  for (auto id : rv::iota(0, static_cast<int>(simulations.size())))
    analysis_file << std::regex_replace(
        std::string{ "geom_line(data = gsdf($), aes(x=update, y = avg, "
                     "color=exp_labels[$])) +\n  geom_errorbar(data = gsdf($), "
                     "aes(x=update,ymin = avg-avgsd, ymax = avg+avgsd, "
                     "color=exp_labels[$]), width = 0.2, alpha=0.2) +\n" },
        std::regex{ "\\$" },
        std::to_string(id + 1));
  analysis_file << "scale_color_discrete(name = \"Conditions\")\ndev.off()\n";
}

}   // namespace experiments
}   // namespace ded
