
#include "exit_room.h"

#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <range/v3/all.hpp>

double
    exit_room::eval(life::entity &org)
{

  auto score = 0.0;

  std::vector<clone> org_copies = ranges::view::iota(0u, num_copies_) |
                                  ranges::view::transform([&](auto i) {
                                    return clone{ org, i, i };
                                  });

  std::vector<std::array<size_t,4>> exit_orders;

  for (auto trial : ranges::view::iota(0,num_trials_))  
  {
    auto in_room = org_copies;
    ranges::shuffle(in_room);
	for(auto i : ranges::view::iota(0u, num_copies_))
		in_room[i].position = i;
	std::vector<size_t> room_org_counts(num_copies_, 1u);

    for (auto i = 0u; i < updates_; i++)
    {
      // feed input to org; inputs are 0s and 1s only
      auto next_org = std::rand() % in_room.size();
	  //auto right= next_org == num_copies_ - 1 ? {0,1} : {room_org_counts[next_org+1],0};

      in_room[next_org].e.input(
          org_input_vision_tag_,
          std::vector<double>{
              next_org ? 0. : 1.,
              next_org ? room_org_counts[next_org - 1] : 0.,
              static_cast<double>(room_org_counts[next_org] - 1),
              next_org == num_copies_ - 1 ? room_org_counts[next_org + 1] : 0.,
              next_org == num_copies_ - 1 ? 1. : 0. });

      // run the org once
      org.tick();
      // read its outputs and interact with the environment
      auto output =
          std::get<std::vector<double>>(org.output(org_output_action_tag_));
      if (output.size() != 1)
      {
        std::cout << "Error: environment-edlund_maze must recieve an output of "
                     "size 2\n";
        exit(1);
    }

    // outputs are interpreted as 0s and 1s only
    auto turn = util::Bit(output[0]);

	}
	}
  return score;
}

life::population
    exit_room::evaluate(life::population pop)
{
  auto vec = pop.get_as_vector();
  // generate new maze
  for (auto &org : vec) org.data.set_value(score_tag_, eval(org));
  pop.merge(vec);
  return pop;
}
