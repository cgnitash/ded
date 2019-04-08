
#include "null_entity.h"

#include <algorithm>
#include <vector>

void
    null_entity::reset()
{
}
void
    null_entity::mutate()
{
}
void         null_entity::input(std::string, life::signal) {}
life::signal null_entity::output(std::string) { return life::signal(); }
void
    null_entity::tick()
{
}
