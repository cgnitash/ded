
#include "edlund_maze.h"

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

double
    edlund_maze::eval(life::entity &org)
{

  auto score             = 0.0;
  auto corridor_num      = 0u;
  auto corridor_position = 0u;
  auto facing_up         = true;

  for (auto i = 0u; i < updates_; i++)
  {
    // feed input to org; inputs are 0s and 1s only
    auto on_door = door_positions_[corridor_num] == corridor_position;
    auto next_door =
        door_positions_[corridor_num + 1] > door_positions_[corridor_num];
    org.input(org_input_vision_tag_,
              std::vector<double>{
                  facing_up
                      ? !corridor_position ? 1. : 0.
                      : corridor_position == corridor_length_ - 1 ? 1. : 0.,
                  on_door ? next_door : 0.,
                  on_door ? !next_door : 0. });

    // run the org once
    org.tick();

    // read its outputs and interact with the environment
    auto output =
        std::get<std::vector<double>>(org.output(org_output_action_tag_));

    // outputs are interpreted as 0s and 1s only
    auto turn = util::Bit(output[0]);
    auto jump = util::Bit(output[1]);
    if (jump && door_positions_[corridor_num] == corridor_position)
    {
      corridor_num++;
      score++;
      if (corridor_num == doors_) break;   // return score + updates_ - i;
    } else if (turn)
      facing_up = !facing_up;
    else
      corridor_position +=
          facing_up ? corridor_position ? -1 : 0
                    : corridor_position == corridor_length_ - 1 ? 0 : 1;
  }

  return score;
}

life::population
    edlund_maze::evaluate(life::population pop)
{
  auto vec = pop.get_as_vector();
  // generate new maze
  ranges::generate_n(ranges::begin(door_positions_), doors_, [this] {
    return std::rand() % corridor_length_;
  });
  for (auto &org : vec)
  {
    org.reset();
    org.data.set_value(distance_tag_, eval(org));
  }
  pop.merge(vec);
  return pop;
}
