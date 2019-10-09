#pragma once

#include "../../../components.hpp"

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class forager_bf
{

  std::string food_eaten_tag_ = "double";

  std::string org_front_input_los_tag_ = "<double,sensor_range>";
  std::string org_back_input_los_tag_  = "<double,sensor_range>";
  std::string org_output_action_tag_   = "<double,2>";

  long   grid_size_    = 10;
  long   sensor_range_ = 4;
  long   updates_      = 100;
  bool   replace_      = true;
  bool   directional_  = true;
  bool   visualize_    = false;
  double density_      = 0.1;

  struct location
  {
    size_t x_, y_;
  };
  enum class direction
  {
    up,
    left,
    down,
    right
  };

  std::vector<std::vector<int>> resources_;

  direction
      turn(direction d, long rate)
  {
    ded::utilities::repeat(rate, [&] {
      d = d == direction::up ? direction::left
                             : d == direction::down
                                   ? direction::right
                                   : d == direction::left ? direction::down :
                                                          /* direction::right */
                                         direction::up;
    });
    return d;
  }

  location
      wrap(location p)
  {
    return { (p.x_ + grid_size_) % grid_size_,
             (p.y_ + grid_size_) % grid_size_ };
  }

  location
      move_in_dir(location p, direction d)
  {
    return d == direction::up
               ? wrap({ p.x_ - 1, p.y_ })
               : d == direction::down
                     ? wrap({ p.x_ + 1, p.y_ })
                     : d == direction::left ? wrap({ p.x_, p.y_ - 1 })
                                            /* direction::right */
                                            : wrap({ p.x_, p.y_ + 1 });
  }

  void   replace_resource_();
  void   initialize_resource_();
  void   interact(ded::concepts::Signal, location &, direction &, double &);
  double eval(ded::concepts::Substrate &);
  std::vector<double> signals_at(location, direction);
  std::vector<double> signals_at(const location);
  void                visualize(std::ofstream &, location, direction, double);

public:
  forager_bf()
  {
    configure(publishConfiguration());
  }

  ded::specs::ProcessSpec
      publishConfiguration()
  {
    ded::specs::ProcessSpec es;   //{"forager"};
    es.bindParameter("grid_size", grid_size_);
    es.bindParameter("updates", updates_);
    es.bindParameter("density", density_);
    es.bindParameter("replace", replace_);
    es.bindParameter("visualize", visualize_);
    es.bindParameter("sensor_range", sensor_range_);
    es.bindParameter("direction", directional_);

    es.bindPostTag("food_eaten", "double");

    es.bindInput("front_line_of_sight", "<double,sensor_range>");
    es.bindInput("back_line_of_sight", "<double,sensor_range>");
    es.bindOutput("action", "<double,2>");
    return es;
  }

  void
      configure(ded::specs::ProcessSpec es)
  {

    es.configureParameter("grid_size", grid_size_);
    es.configureParameter("updates", updates_);
    es.configureParameter("density", density_);
    es.configureParameter("replace", replace_);
    es.configureParameter("visualize", visualize_);
    es.configureParameter("sensor_range", sensor_range_);
    es.configureParameter("direction", directional_);

    resources_ = std::vector(grid_size_, std::vector(grid_size_, 0));

    es.configurePostTag("food_eaten", food_eaten_tag_);

    es.configureInput("front_line_of_sight", org_front_input_los_tag_);
    es.configureInput("back_line_of_sight", org_back_input_los_tag_);
    es.configureOutput("action", org_output_action_tag_);
  }

  ded::concepts::Population evaluate(ded::concepts::Population);
};
