
#include"../../components.h"
#include"../../core/utilities.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <regex>
#include <set>
#include <map>
#include <initializer_list>
#include <unordered_map>
#include <unordered_set>

class forager {

  size_t grid_size_ = 10;
  size_t sensor_range_ = 4;
  size_t updates_ = 100;
  bool replace_ = true;
  double density_ = 0.1;

  
  struct location {
    size_t x_, y_;
  };
  enum class direction { up,  left, down, right };

  std::vector<std::vector<int>> resources_;

  direction turn(direction d, long rate) {
    util::repeat(rate, [&] {
      d = d == direction::up ? direction::right
                             : d == direction::down
                                   ? direction::left
                                   : d == direction::left ? direction::up :
                                                          /* direction::right */
                                         direction::down;
    });
    return d;
  }

  location wrap(location p) { return {p.x_ % grid_size_, p.y_ % grid_size_}; }


  location move_in_dir(location p, direction d) {
    return d == direction::up ? wrap({p.x_ + grid_size_ - 1, p.y_})
                              : d == direction::down
                                    ? wrap({p.x_ + 1, p.y_})
                                    : d == direction::left
                                          ? wrap({p.x_, p.y_ + grid_size_ - 1})
                                          /* direction::right */
                                          : wrap({p.x_, p.y_ + 1});
  }

  void replace_resource_();
  void initialize_resource_();
  void interact(life::signal, location &, direction &, double &);
  double eval(life::entity&);
  std::vector<double> signals_at(location,direction);

public:
  forager() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["grid-size"] = grid_size_;
    ec["updates"] = updates_;
    ec["density"] = density_;
    ec["replace"] = replace_;
    ec["sensor-range"] = sensor_range_	;
    return ec;
  }

  void configure(life::configuration con) {

    grid_size_ = con["grid-size"];
    updates_= con["updates"];
    density_ = con["density"];
    replace_ = con["replace"];
    sensor_range_	= con["sensor-range"];
    resources_ = std::vector(grid_size_, std::vector(grid_size_, 0));
  }

  life::population evaluate(life::population);
};

