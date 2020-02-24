

#pragma once

#include <map>
#include <string>
#include <any>
#include <vector>

namespace ded
{
namespace concepts
{

using Signal = std::any;
/*
	std::variant<long,
                            double,
                            bool,
                            std::vector<long>,
                            std::vector<double>,
                            std::vector<bool>>;
*/

/*
struct NamedS
{
	std::string name_;
	Signal signal_;
};
*/


class DataStore
{

  std::map<std::string, Signal> m;

public:
  Signal
      getValue(std::string n) const
  {
    return m.at(n);
  }
  auto
      setValue(std::string n, Signal v)
  {
    m[n] = v;
  }
  size_t
      size() const
  {
    return m.size();
  }
  void
      clear(std::string i)
  {
    m.erase(i);
  }
  void
      merge(DataStore d)
  {
    m.insert(d.m.begin(), d.m.end());
  }
};
}   // namespace concepts
}   // namespace ded
