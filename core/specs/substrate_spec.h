
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../language/parser.h"
#include "../utilities/term_colours.h"
#include "../utilities/utilities.h"
#include "configuration_primitive.h"
#include "signal_spec.h"
#include "encoding_spec.h"

namespace ded
{

// forward declaration to provide friendship
namespace concepts
{
class Substrate;
}

namespace specs
{

class SubstrateSpec
{
  struct NestedSubstrateSpec
  {
    std::unique_ptr<SubstrateSpec> e;
    NestedSubstrateSpec() = default;
    NestedSubstrateSpec(const NestedSubstrateSpec &ns)
        : e(std::make_unique<SubstrateSpec>(*ns.e))
    {
    }
  };

  std::string                                   name_;
  std::map<std::string, ConfigurationPrimitive> parameters_;
  std::map<std::string, NestedSubstrateSpec>             nested_;
  std::map<std::string, std::vector<NestedSubstrateSpec>>             nested_vector_;
  std::map<std::string, EncodingSpec>                    encodings_;
  IO                                            io_;

public:

  SubstrateSpec(std::string name = "") : name_(name)
  {
  }
  SubstrateSpec(language::Parser, language::Block);

  auto
      name() const
  {
    return name_;
  }

  auto
      getIO()
  {
    return io_;
  }

  template <typename T>
  void
      bindParameter(
          std::string                                                 name,
          T                                                           value,
          std::vector<std::pair<std::function<bool(T)>, std::string>> cons = {})

  {
    if (parameters_.find(name) != parameters_.end())
    {
      std::cout << "User error: parameter " << name
                << " has already been declared\n";
      throw SpecError{};
    }
    parameters_[name].setValue(value);
    parameters_[name].setConstraints(cons);
  }

  template <typename T>
  void
      configureParameter(std::string name, T &value)
  {
    if (parameters_.find(name) == parameters_.end())
    {
      std::cout << "User error: parameter " << name
                << " has not been declared\n";
      throw SpecError{};
    }
    parameters_[name].get_value(value);
  }

  void
      bindInput(std::string name, std::string value)
  {
    io_.inputs_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configureInput(std::string name, std::string &value)
  {
    auto i =
        rs::find_if(io_.inputs_, [=](auto ns) { return ns.first == name; });
    value = i->second.identifier();
  }

  void
      bindOutput(std::string name, std::string value)
  {
    io_.outputs_.push_back({ name, SignalSpec{ name, name, value } });
  }

  void
      configureOutput(std::string name, std::string &value)
  {
    auto i = rs::find_if(io_.outputs_,
                             [=](auto ns) { return ns.first == name; });
    value  = i->second.identifier();
  }

  void
      bindEncoding(std::string name, EncodingSpec e)
  {
    encodings_[name] = e;
  }

  void
      configureEncoding(std::string name, EncodingSpec &e)
  {
      e = encodings_[name];
  }

  void
      bindNestedSubstrates(std::string name, std::vector<SubstrateSpec> subs)
  {
    if (nested_vector_.find(name) != nested_vector_.end())
    {
      std::cout << "User error: nested vector substrate " << name
                << " has already been declared\n";
      throw SpecError{};
    }
	nested_vector_[name];
    for (auto sub : subs)
    {
      NestedSubstrateSpec ns;
      ns.e = std::make_unique<SubstrateSpec>(sub);
      nested_vector_[name].push_back(ns);
    }
  }

  void
      configureNestedSubstrates(std::string                 name,
                                std::vector<SubstrateSpec> &subs)
  {
    if (nested_vector_.find(name) == nested_vector_.end())
    {
      std::cout << "User error: nested vector substrate " << name
                << " has not been declared\n";
      throw SpecError{};
    }
    subs.clear();
    for (auto ns : nested_vector_[name])
    {
      auto ne = *ns.e;
      subs.push_back(ne);
    }
  }

  void
      bindSubstrate(std::string name, SubstrateSpec sub)
  {
    nested_[name].e = std::make_unique<SubstrateSpec>(sub);
  }

  void
      configureSubstrate(std::string name, SubstrateSpec &sub)
  {
    if (!nested_[name].e)
    {
      std::cout << "Warning: <" << name_ << ":" << name
                << "> environment spec has not been bind-ed (probably error)\n";
      //      std::exit(1);
    }
    else
      sub = *nested_[name].e;
  }

  void parseParameters(language::Parser , language::Block);
  void parseNested(language::Parser , language::Block);
  void parseNestedVector(language::Parser , language::Block);

  // friend std::ostream &operator<<(std::ostream &out, SubstrateSpec e)
  std::vector<std::string> serialise(long) const;
  SubstrateSpec  deserialise(std::vector<std::string> );
  std::string prettyPrint();

  IO instantiateUserParameterSizes(int);

  void
      bindSubstrateIO(IO ios)
  {
    io_.bindTo(ios);
  }



  friend class concepts::Substrate;
};

}   // namespace specs
}   // namespace ded
