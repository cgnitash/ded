

#pragma once

#include <string>
#include <variant>
#include <vector>

namespace life {

using signal = std::variant<long, double, std::string, std::vector<double>,
                            std::vector<long>, std::vector<std::string>>;

}
