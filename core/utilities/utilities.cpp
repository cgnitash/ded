

#include <range/v3/all.hpp>
#include <cmath>
#include <experimental/filesystem>
#include <fstream>

#include "utilities.h"

namespace ded {
namespace utilities {
double
    Bit(double d)
{
  return d > 0. ? 1. : 0.;
}

std::ofstream
    open_or_append(std::string file_name, std::string header)
{
  std::ofstream file;
  if (!std::experimental::filesystem::exists(file_name))
  {
    file.open(file_name);
    file << header;
  } else
    file.open(file_name, std::ios::app);
  return file;
}

}   // namespace util
}   // namespace util
