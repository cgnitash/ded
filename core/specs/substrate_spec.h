
#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <variant>

#include "../language/parser.h"
#include "../language/token.h"
#include "../utilities/term_colours.h"
#include "../utilities/utilities.h"
#include "configuration_primitive.h"
#include "encoding_spec.h"
#include "signal_spec.h"

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

  std::string                                             name_;
  language::Token                                         name_token_;
  std::map<std::string, ConfigurationPrimitive>           parameters_;
  std::map<std::string, NestedSubstrateSpec>              nested_;
  std::map<std::string, std::vector<NestedSubstrateSpec>> nested_vector_;
  std::map<std::string, EncodingSpec>                     encodings_;
  IO                                                      io_;

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
      nameToken() const
  {
    return name_token_;
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

  void bindInput(std::string name, std::string value);

  void configureInput(std::string name, std::string &value);
  void bindOutput(std::string name, std::string value);
  void configureOutput(std::string name, std::string &value);
  void bindEncoding(std::string name, EncodingSpec e);
  void configureEncoding(std::string name, EncodingSpec &e);
  void bindNestedSubstrates(std::string name, std::vector<SubstrateSpec> subs);
  void configureNestedSubstrates(std::string                 name,
                                 std::vector<SubstrateSpec> &subs);
  void bindSubstrate(std::string name, SubstrateSpec sub);
  void configureSubstrate(std::string name, SubstrateSpec &sub);
  void parseParameters(language::Parser, language::Block);
  void parseNested(language::Parser, language::Block);
  void parseNestedVector(language::Parser, language::Block);

  // friend std::ostream &operator<<(std::ostream &out, SubstrateSpec e)
  std::vector<std::string> serialise(long) const;
  SubstrateSpec            deserialise(std::vector<std::string>);
  std::string              prettyPrint();

  void instantiateUserParameterSizes(int);

  void
      bindSubstrateIO(SubstrateSpec);

  friend class concepts::Substrate;
};

}   // namespace specs
}   // namespace ded
