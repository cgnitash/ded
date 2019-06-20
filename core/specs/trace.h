
#pragma once

#include<string>
#include<vector>

namespace life {
    struct trace
    {
      std::string name_;
      std::string type_;
      int         frequency_;
    };

  struct trace_config 
  {
    std::vector<trace> pre_;
    std::vector<trace> post_;
  } ; 
}
