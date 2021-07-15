#pragma once
#include <string>

namespace ded
{
namespace utilities
{
// convenience struct for printing out colours
struct TermColours
{
  static const std::string black_fg, red_fg, green_fg, yellow_fg, blue_fg,
      magenta_fg, cyan_fg, white_fg, black_bg, red_bg, green_bg, yellow_bg,
      blue_bg, magenta_bg, cyan_bg, white_bg, reset, bold_on, underline_on,
      inverse_on, bold_off, underline_off, inverse_off;
};
}   // namespace utilities
}   // namespace ded
