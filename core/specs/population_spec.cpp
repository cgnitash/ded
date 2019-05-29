
#include <iomanip>
#include <iostream>
#include <map>
#include <range/v3/all.hpp>
#include <regex>
#include <string>
#include <variant>


#include "entity_spec.h"
#include "population_spec.h"

namespace life {
  std::string population_spec::dump(long depth)
  {
    auto alignment = "\n" + std::string(depth, ' ');

    auto pad = [&] {
      return ranges::view::transform(
                 [&](auto p) { return alignment + p.first + ":" + p.second; }) |
             ranges::action::join;
    };

    return alignment + "population:" + name_ + alignment + "P" + 
           (parameters_ | ranges::view::transform([&](auto parameter) {
              return alignment + parameter.first + ":" +
                     parameter.second.value_as_string();
            }) |
            ranges::action::join) +
           alignment + "I" + (inputs_ | pad()) + alignment + "O" +
           (outputs_ | pad()) + alignment + "E" + es_.dump(depth + 1);
  }

  population_spec
      population_spec::parse(std::vector<std::string> pop_dump)
  {
	name_ =     *pop_dump.begin();
	name_ = name_.substr(name_.find(':')+1);

    auto            f = ranges::begin(pop_dump) + 2;

    for (; *f != "I"; f++)
    {
      auto                          l = *f;
      auto                          p = l.find(':');
      life::configuration_primitive c;
      c.parse(l.substr(p + 1));
	  //std::cout << c.value_as_string() << std::endl
	//	  << l.substr(0, p) << ";" << std::endl
//		  << l.substr(p + 1) <<";" <<  std::endl;
      parameters_[l.substr(0, p)] = c;
	}

    for (; *f != "O"; f++)
    {
      auto                          l = *f;
      auto                          p = l.find(':');
      inputs_[l.substr(0, p)] = l.substr(p + 1);
	}

    for (; *f != "E"; f++)
    {
      auto                          l = *f;
      auto                          p = l.find(':');
      outputs_[l.substr(0, p)] = l.substr(p + 1);
	}

    for (auto l = (++f) + 1; l != pop_dump.end(); l++)
    {
		l->erase(0,1);
	}

	entity_spec es;
	es_ =  es.parse(std::vector<std::string>(f, pop_dump.end()));

    population_spec ps = *this;
    return ps;
  }

  std::string population_spec::pretty_print()
  {
    std::stringstream out;
    out << term_colours::red_fg << "population::" << name_
        << term_colours::reset << "\n";

    out << term_colours::yellow_fg << "parameters----" << term_colours::reset
        << "\n";
    for (auto [parameter, value] : parameters_)
      out << std::setw(26) << parameter << " : " << value.value_as_string()
          << "\n";
    out << term_colours::yellow_fg << "inputs----" << term_colours::reset
        << "\n";
    for (auto [input, value] : inputs_)
      out << std::setw(26) << input << " : " << value << "\n";
    out << term_colours::yellow_fg << "outputs----" << term_colours::reset
        << "\n";
    for (auto [output, value] : outputs_)
      out << std::setw(26) << output << " : " << value << "\n";

    return out.str();
  }  }
