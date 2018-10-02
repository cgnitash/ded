
#include "entity.h"

# pragma once 

#include <vector>
#include <map>
#include <string>

namespace life {

using eval_results =
    std::vector<std::pair<life::entity, std::map<std::string, std::string>>>;
}
