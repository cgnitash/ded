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

  std::string food_eaten_tag_ = "food-eaten,double";

  std::string org_input_los_tag_ = "line-of-sight,A<double,sensor-range>";
  std::string org_output_action_tag_ = "action,A<double,2>";

  size_t grid_size_ = 10;
  size_t sensor_range_ = 4;
  size_t updates_ = 100;
  bool replace_ = true;
  bool visualize_ = false;
  double density_ = 0.1;

  struct location {
    size_t x_, y_;
  };
  enum class direction { up, left, down, right };

  std::vector<std::vector<int>> resources_;

  direction turn(direction d, long rate) {
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

  location wrap(location p) {
    return {(p.x_ + grid_size_) % grid_size_, (p.y_ + grid_size_) % grid_size_};
  }

  location move_in_dir(location p, direction d) {
    return d == direction::up
               ? wrap({p.x_ - 1, p.y_})
               : d == direction::down
                     ? wrap({p.x_ + 1, p.y_})
                     : d == direction::left ? wrap({p.x_, p.y_ - 1})
                                            /* direction::right */
                                            : wrap({p.x_, p.y_ + 1});
  }

  void replace_resource_();
  void initialize_resource_();
  void interact(life::signal, location &, direction &, double &);
  double eval(life::entity &);
  std::vector<double> signals_at(location, direction);
  void visualize(std::ofstream &, location, direction, double);

public:
  forager() { configure(publish_configuration()); }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["parameters"]["grid-size"] = grid_size_;
    ec["parameters"]["updates"] = updates_;
    ec["parameters"]["density"] = density_;
    ec["parameters"]["replace"] = replace_;
    ec["parameters"]["visualize"] = visualize_;
    ec["parameters"]["sensor-range"] = sensor_range_;

    // o:in:P has no tags
    ec["pre-tags"] = nullptr;

    //  o:in:P' must handle these tags
    ec["post-tags"]["food-eaten"] = food_eaten_tag_;

	ec["org-inputs"]["line-of-sight"] = org_input_los_tag_; 
	ec["org-outputs"]["action"] = org_output_action_tag_; 
    return ec;
  }

  void configure(life::configuration con) {

    grid_size_ = con["parameters"]["grid-size"];
    updates_ = con["parameters"]["updates"];
    density_ = con["parameters"]["density"];
    replace_ = con["parameters"]["replace"];
    visualize_ = con["parameters"]["visualize"];
    sensor_range_ = con["parameters"]["sensor-range"];

    resources_ = std::vector(grid_size_, std::vector(grid_size_, 0));

    food_eaten_tag_ = con["post-tags"]["food-eaten"];

    org_input_los_tag_ = con["org-inputs"]["line-of-sight"];
    org_output_action_tag_ = con["org-outputs"]["action"];
  }

  // requires org.input("sensors","vector<bool,sensor-range>")
  // requires org.output("flipped-bits","vector<bool,size>")
  life::population evaluate(life::population);
  // guarantess org.data["food-eaten"] exists and is integer
};

