
#include"null_entity.h"

#include <vector>
#include <algorithm>

  void null_entity::mutate() {}
  void null_entity::input(std::string, life::signal) {}
  life::signal null_entity::output(std::string) { return life::signal(); }
  void null_entity::tick() {}


