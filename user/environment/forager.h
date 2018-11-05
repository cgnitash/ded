
#include"../../components.h"
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
    bool operator<(const location &A) const{
      return std::tie(x_, y_) < std::tie(A.x_, A.y_);
    }
  };

  enum class direction { up,  left, down, right };
  std::map<location,size_t> signal_strength_;
  std::set<location> resources_;

  direction turn(direction d, long rate) {
    return static_cast<direction>((static_cast<long>(d) + rate) % 4);
  }

  location wrap(location p) { return {p.x_ % grid_size_, p.y_ % grid_size_}; }

  std::initializer_list<location> neighbours(location p) {
    auto ret = {
        p, move_in_dir(p, direction::up), move_in_dir(p, direction::down),
        move_in_dir(p, direction::left), move_in_dir(p, direction::right)};
    return ret;
  }

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
  void refresh_signals();
  void interact(life::signal, location &, direction &, double &);
  double eval(life::entity&);
  std::vector<double> signals_at(location);

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

    grid_size_ = (con["grid-size"]);
    updates_= (con["updates"]);
    density_ = (con["density"]);
    replace_ = (con["replace"]);
    sensor_range_	= (con["sensor-range"]);

  }

  std::vector<life::entity> evaluate(const std::vector<life::entity> &);
};

