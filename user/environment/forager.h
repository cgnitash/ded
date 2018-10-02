
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

  long grid_size_;
  long updates_;
  double density_;
  struct location {
    long x_, y_;
    bool operator<(const location &A) const{
      return std::tie(x_, y_) < std::tie(A.x_, A.y_);
    }
  };

  enum class direction { up,  left, down, right };
  std::map<location,long> signal_strength_;
  std::set<location> resources_;

  direction turn(direction d, long rate) {
    return static_cast<direction>((static_cast<long>(d) + rate) % 4);
  }

  location wrap(location p) {
    return {(p.x_ + grid_size_) % grid_size_, (p.y_ + grid_size_) % grid_size_};
  }

  std::initializer_list<location> neighbours(location p) {
    return {move_in_dir(p, direction::up), move_in_dir(p, direction::down),
            move_in_dir(p, direction::left), move_in_dir(p, direction::right)};
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
  void refresh_signals();
  double eval(life::entity);


        public:
  forager() {
	  configure(publish_configuration());
  }

  life::configuration publish_configuration() {
    life::configuration ec;
    ec["grid-size"] = 10;
    ec["updates"] = 100;
    ec["density"] = 0.1;
    return ec;
  }

  void configure(life::configuration con) {

    grid_size_ = (con["grid-size"]);
    updates_= (con["updates"]);
    density_ = (con["density"]);

  }

  life::eval_results evaluate(const std::vector<life::entity> &);
  };

