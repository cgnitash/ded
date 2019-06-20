#pragma once

#include "../../components.h"

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

class forager {

  std::string food_eaten_tag_ = "food_eaten,double";

  std::string org_input_los_tag_     = "<double,sensor_range>";
  std::string org_output_action_tag_ = "<double,2>";

  long grid_size_    = 10;
  long sensor_range_ = 4;
  long updates_      = 100;
  bool   replace_      = true;
  bool   directional_  = true;
  bool   visualize_    = false;
  double density_      = 0.1;

  struct location
  {
    size_t x_, y_;
  };
  enum class direction { up, left, down, right };

  std::vector<std::vector<int>> resources_;

  direction turn(direction d, long rate)
  {
    util::repeat(rate, [&] {
      d = d == direction::up ? direction::left
                             : d == direction::down
                                   ? direction::right
                                   : d == direction::left ? direction::down :
                                                          /* direction::right */
                                         direction::up;
    });
    return d;
  }

  location wrap(location p)
  {
    return { (p.x_ + grid_size_) % grid_size_,
             (p.y_ + grid_size_) % grid_size_ };
  }

  location move_in_dir(location p, direction d)
  {
    return d == direction::up
               ? wrap({ p.x_ - 1, p.y_ })
               : d == direction::down
                     ? wrap({ p.x_ + 1, p.y_ })
                     : d == direction::left ? wrap({ p.x_, p.y_ - 1 })
                                            /* direction::right */
                                            : wrap({ p.x_, p.y_ + 1 });
  }

  void                replace_resource_();
  void                initialize_resource_();
  void                interact(life::signal, location &, direction &, double &);
  double              eval(life::entity &);
  std::vector<double> signals_at(location, direction);
  std::vector<double> signals_at(const location);
  void                visualize(std::ofstream &, location, direction, double);

public:
  forager() { configure(publish_configuration()); }

  life::environment_spec publish_configuration()
  {
    life::environment_spec es{"forager"};
    es.bind_parameter("grid_size",grid_size_);
    es.bind_parameter("updates",updates_);
    es.bind_parameter("density",density_);
    es.bind_parameter("replace",replace_);
    es.bind_parameter("visualize",visualize_);
    es.bind_parameter("sensor_range",sensor_range_);
    es.bind_parameter("direction",directional_);
 
    es.bind_post("food_eaten",food_eaten_tag_);

    //es.bind_input("line_of_sight",org_input_los_tag_);
    //es.bind_output("action",org_output_action_tag_);
    es.bind_input("line_of_sight","<double,sensor_range>");
    es.bind_output("action","<double,2>");
    return es;
  }

  void configure(life::environment_spec es)
  {

    es.configure_parameter("grid_size",grid_size_);
    es.configure_parameter("updates",updates_);
    es.configure_parameter("density",density_);
    es.configure_parameter("replace",replace_);
    es.configure_parameter("visualize",visualize_);
    es.configure_parameter("sensor_range",sensor_range_);
    es.configure_parameter("direction",directional_);

    resources_ = std::vector(grid_size_, std::vector(grid_size_, 0));

    es.configure_post("food_eaten",food_eaten_tag_);

    es.configure_input("line_of_sight",org_input_los_tag_);
    es.configure_output("action",org_output_action_tag_);
  }

  life::population evaluate(life::population);
};
