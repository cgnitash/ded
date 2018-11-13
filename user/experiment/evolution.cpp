
#include "evolution.h"
#include "../../core/utilities.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

void evolution::run() {

  // generate the population
  std::vector<life::entity> pop;
  util::rv3::generate_n(util::rv3::back_inserter(pop), pop_size_, [&] {
    auto org = life::make_entity(org_name_);
    org.configure(org_config_);
    return org;
  });

  for (auto &org : pop)
  std::cout << org.get_id() << ":";
  std::cout << std::endl; 

  auto optimiser = life::make_environment(sel_name_);
  optimiser.configure(sel_config_);

  auto world = life::make_environment(world_name_);
  world.configure(world_config_);

  std::ofstream file("data.csv");
  file << "avg,max\n";

  for (auto i : util::rv3::view::iota(0, generations_)) {

    pop = world.evaluate(pop);

  for (auto i : pop | util::rv3::view::transform([](auto const &org) {
			 return org.get_id(); 
			 }))
  std::cout << i << " ";
  std::cout << std::endl; 
    const auto scores = pop | util::rv3::view::transform([](auto const &org) {
                    return double{org.data["score"]};
                  });

    const auto avg = util::rv3::accumulate(scores, 0.0) / pop.size();

    const auto max = *util::rv3::max_element(scores);

    if (!(i % 100))
      std::cout << "update: " << std::setw(3) << i << " avg: " << std::setw(3)
                << std::setprecision(2) << avg << "   max: " << std::setw(3)
                << std::setprecision(2) << max << std::endl;

    file << avg << "," << max << '\n';

    pop = optimiser.evaluate(pop);
  for (auto i : pop | util::rv3::view::transform([](auto const &org) {
			 return org.get_id(); 
			 }))
  std::cout << i << " ";
  std::cout << std::endl; 
  }
}

