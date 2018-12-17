

#pragma once

#include <string>
#include <variant>
#include <initializer_list>
#include <vector>

namespace life {

using signal = std::variant<long, double, std::string, std::vector<double>,
                            std::vector<long>, std::vector<std::string>,
                            std::initializer_list<double>>;
}
