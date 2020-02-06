
#pragma once

#include <string>
#include <vector>

#include "../utilities/term_colours.hpp"

namespace ded
{
namespace specs
{

class SignalSpec
{
private:
  std::string user_name_{};
  std::string internal_identifier_{};
  std::string full_type_{};

  std::string vector_type_{};
  std::string user_parameter_{};
  long        size_ = 0;
  bool        is_any_type_             = false;
  bool        is_vector_               = false;
  bool        is_any_vector_size_      = false;
  bool        is_user_set_vector_size_ = false;
  bool        is_explicitly_bound_     = false;

public:
  SignalSpec() = default;

  SignalSpec(std::string sig);

  SignalSpec(std::string name, std::string idt);

  // should be the only constructor
  SignalSpec(std::string name, std::string id, std::string type);

  auto
      fullName() const
  {
    return user_name_ + ":" + internal_identifier_ + "-" + full_type_;
  }

  auto
      diagnosticName() const
  {
    return user_name_ + " : " + full_type_ +
           (is_user_set_vector_size_
                ? utilities::TermColours::blue_fg + "    with [" +
                      user_parameter_ + " = " + std::to_string(size_) + "]" +
                      utilities::TermColours::reset
                : "");
  }

  auto
      userName() const
  {
    return user_name_;
  }

  auto
      identifier() const
  {
    return internal_identifier_;
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

  void
      updateIdentifier(std::string id)
  {
    internal_identifier_ = id;
  }

  void
      instantiateUserParameter(long size)
  {
    size_ = size;
  }

  bool exactlyMatches(SignalSpec);
  bool convertibleTo(SignalSpec);
  bool isExplicitlyBound() { return is_explicitly_bound_; }
  void setExplicitlyBound() { is_explicitly_bound_ = true; }
};

struct NamedSignal
{
	std::string name_{};
	SignalSpec signal_spec_{};
};

struct SignalConstraint
{
	std::string name_;
    std::string type_;
};

NamedSignal
    toSignal(SignalConstraint user_constraint);

using SignalSpecSet = 
  std::vector<NamedSignal>;

// struct io_signals
struct IO
{
	public :
  SignalSpecSet inputs_;
  SignalSpecSet outputs_;
//  void bindTo(SubstrateSpec, std::string);
};

struct Tags
{
	public :
  SignalSpecSet pre_;
  SignalSpecSet post_;
};

enum class TagType
{ pre, post
};

struct NamedTag
{
	std::string name_;
	TagType type_;
};

struct TagFlow
{
	NamedTag from_;
	NamedTag to_;
};
}   // namespace specs
}   // namespace ded
