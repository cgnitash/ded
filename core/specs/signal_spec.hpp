
#pragma once

#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "../utilities/term_colours.hpp"
#include "../utilities/utilities.hpp"

namespace ded
{
namespace specs
{

inline static const std::regex valid_signal_type_{
  R"~~((\w+)?|<(\w+)?,(\w+|\d+|_)?>)~~"
};

class SignalSpec
{
private:
  std::string full_type_{};
  std::string vector_type_{};
  std::string user_parameter_{};
  long        size_                       = 0;
  bool        is_any_type_                = false;
  bool        is_vector_                  = false;
  bool        is_any_vector_size_         = false;
  bool        is_user_set_vector_size_    = false;
  bool        is_placeholder_vector_size_ = false;

  struct
  {
    bool             is_bound_ = false;
    std::vector<int> indices_;
  } bindings_;

public:
  SignalSpec() = default;

  SignalSpec(std::string type);

  auto
      diagnosticName() const
  {
    return full_type_ +
           (is_user_set_vector_size_
                ? utilities::TermColours::blue_fg + "    with [" +
                      user_parameter_ + " = " + std::to_string(size_) + "]" +
                      utilities::TermColours::reset
                : "");
  }

  auto
      type() const
  {
    return full_type_;
  }

  auto
      userParameter() const
  {
    return user_parameter_;
  }

  void instantiateUserParameter(long size);

  void updatePlaceholders(SignalSpec);
  bool exactlyMatches(SignalSpec);
  bool convertibleTo(SignalSpec);

  bool
      isVectorType() const
  {
    return is_vector_;
  }

  bool
      isBound() const
  {
    return bindings_.is_bound_;
  }

  void
      setBound()
  {
    bindings_.is_bound_ = true;
    bindings_.indices_.clear();
  }

  void addBoundIndices(std::vector<int> indices);

  std::vector<int>
      unboundIndices()
  {
    return bindings_.indices_;
  }

  bool isPartiallyBounded()
  {
    return bindings_.indices_.size() &&
           static_cast<long>(bindings_.indices_.size()) != size_;
  }
};

struct NamedSignal
{
  std::string name_{};
  SignalSpec  signal_spec_{};
};

struct SignalConstraint
{
  std::string name_;
  std::string type_;
};

NamedSignal toSignal(SignalConstraint user_constraint);

using SignalSpecSet = std::vector<NamedSignal>;

struct IO
{
public:
  SignalSpecSet inputs_;
  SignalSpecSet outputs_;
};

struct Tags
{
public:
  SignalSpecSet pre_;
  SignalSpecSet post_;
};

}   // namespace specs
}   // namespace ded
