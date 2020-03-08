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

class forager {

public:

  void configuration(ded::specs::ProcessSpec &spec)
  {

    spec.parameter("grid_size",grid_size_);
    spec.parameter("updates",updates_);
    spec.parameter("density",density_);
    spec.parameter("replace",replace_);
    spec.parameter("visualize",visualize_);
    spec.parameter("sensor_range",sensor_range_);
    spec.parameter("direction",directional_);

    spec.postTag("food_eaten","double");

    spec.input("line_of_sight","<double,sensor_range>",input_los);
    spec.output("action", "<double,2>", output_action);

    resources_ = std::vector(grid_size_, std::vector(grid_size_, 0));
  }

  ded::concepts::Population evaluate(ded::concepts::Population);

private:
  ded::specs::ConversionSignatureSequence input_los;
  ded::specs::ConversionSignatureSequence output_action;

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
};
