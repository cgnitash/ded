
#include"null_entity.h"

#include <vector>
#include <algorithm>

  void null_entity::mutate() {}
  void null_entity::input(life::signal) {}
  life::signal null_entity::output() { return life::signal(); }
  void null_entity::tick() {}


