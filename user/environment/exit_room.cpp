
#include "exit_room.h"

#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

double
    exit_room::eval(life::entity &org)
{

 // auto score = 0.0;

  std::vector<clone> org_copies = ranges::view::iota(0u, num_copies_) |
                                  ranges::view::transform([&](auto i) {
                                    return clone{ org, i, i };
                                  });

  std::map<std::vector<size_t>, size_t> all_exits;

  for ([[maybe_unused]] auto trial : ranges::view::iota(0, num_trials_))
  {
    auto in_room = org_copies;
    ranges::shuffle(in_room);
    for (auto i : ranges::view::iota(0u, num_copies_)) in_room[i].position = i;
    std::vector<size_t> room_org_counts(num_copies_, 1u);
    std::vector<size_t> exit_orders;

    for (auto i = 0u; i < updates_; i++)
    {
      // feed input to org; inputs are 0s and 1s only
      auto next_org = std::rand() % in_room.size();
      // auto right= next_org == num_copies_ - 1 ? {0,1} :
      // {room_org_counts[next_org+1],0};

      auto &o = in_room[next_org];

      o.e.input(
          org_input_vision_tag_,
          std::vector<double>{
              next_org ? 0. : 1.,
              next_org ? room_org_counts[next_org - 1] : 0.,
              static_cast<double>(room_org_counts[next_org] - 1),
              next_org == num_copies_ - 1 ? room_org_counts[next_org + 1] : 0.,
              next_org == num_copies_ - 1 ? 1. : 0. });

      // run the org once
      o.e.tick();
      // read its outputs and interact with the environment
      auto output =
          std::get<std::vector<double>>(o.e.output(org_output_action_tag_));
      if (output.size() != 1)
      {
        std::cout << "Error: environment-exit_room must recieve an output of "
                     "size 2\n";
        exit(1);
      }

      // outputs are interpreted as 0s and 1s only
      auto turn = util::Bit(output[0]);
      if (turn)
      {
        if (o.position == num_copies_ - 1)
        {
          exit_orders.push_back(o.id);
          in_room.erase(std::begin(in_room) + next_org);
		  if (in_room.empty()) break;
        } else
          o.position++;
      }
    }
	if (exit_orders.size() == num_copies_)
		all_exits[exit_orders]++;
  }
  
  auto cc  = ranges::accumulate(all_exits, 0., [](auto sum, auto exit) {
		 return sum + exit.second; 
		 });
	if (cc != num_trials_) return 0;//std::cout << "hey, agent stuck\n";
  return ranges::accumulate(all_exits, 0., [this](auto sum, auto exit) {
		 auto p = exit.second / static_cast<double>(num_trials_);
		 return sum - p * std::log2(p) / std::log2(num_trials_);
		 });
}

life::population
    exit_room::evaluate(life::population pop)
{
  auto vec = pop.get_as_vector();
  for (auto &org : vec) org.data.set_value(score_tag_, eval(org));
  pop.merge(vec);
  return pop;
}
