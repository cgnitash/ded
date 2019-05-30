#pragma once

#include <type_traits>

// tweaks from http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4502.pdf
namespace enhanced_type_traits {
// Primary template handles all types not supporting the operation.
template <typename, template <typename> typename, typename = std::void_t<>>
struct is_detected : std::false_type
{
};

// // Specialization recognizes/validates only types supporting the archetype.
template <typename T, template <typename> typename Op>
struct is_detected<T, Op, std::void_t<Op<T>>> : std::true_type
{
};

template <typename T, typename Ret, template <typename> typename Op>
struct has_signature
    : std::conjunction<is_detected<T, Op>, std::is_same<Op<T>, Ret>>
{
};

// From https://www.youtube.com/watch?v=9PFMllbyaLM
// template class that contains an overload set of operator()
template <typename... lambdas> struct overload_set : lambdas...
{
  using lambdas::operator()...;
};

// Deduction guide to instantiate the template
template <typename... lambdas>
overload_set(lambdas...)->overload_set<lambdas...>;
}   // namespace enhanced_type_traits
