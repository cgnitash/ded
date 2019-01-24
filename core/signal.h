

#pragma once

#include <string>
#include <variant>
#include <vector>

namespace life {

using signal = std::variant<long,
                            double,
                            bool,
                            std::vector<long>,
                            std::vector<double>,
                            std::vector<bool>>;
}
