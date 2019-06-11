
#pragma once

#include<string>
#include<vector>

namespace life {
    struct trace
    {
      std::string trace_;
      int         frequency_;
    };

  struct trace_config 
  {
    std::vector<trace> pre_;
    std::vector<trace> post_;
  } ; 
}
