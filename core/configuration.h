
# pragma once

#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>

namespace life{

using configuration = nlohmann::json;
using ModuleInstancePair = std::pair<std::string, std::string>;
extern std::map<ModuleInstancePair, life::configuration> all_configs;

inline void validate_subset(const configuration &in, const configuration &real) {
	if (in.empty()) 
		return;
	const auto &in_params = in["parameters"];
	const auto &real_params = real["parameters"];
    for (auto it = in_params.begin(); it != in_params.end(); ++it) {
      if (real_params.find(it.key()) == real_params.end()) {
        std::cout << "Error: Configuration mismatch -- \"" << it.key()
                  << "\" is not a valid parameter\n";
        exit(1);
      }
    }
}

inline void merge_into(configuration &in, const configuration &real) {
	if (in.empty()) 
	{ in = real;
	} else {
	auto& in_params = in["parameters"];
	const auto &real_params = real["parameters"];
  for (const auto &[key, value] : real_params.items()) 
    if (in_params.find(key) == in_params.end()) 
      in_params[key] = value;
	}
}

namespace config {

// In : abc
// Out : [bc,ac,ab]
inline auto deletes(std::string word) {
  return ranges::view::repeat_n(word, word.size()) |
         ranges::view::transform([n = 0](auto str) mutable {
           n++;
           return str.substr(0, n - 1) + str.substr(n);
         });
}

// In : xabc
// Out : [xabc,axbc,abxc,abcx]
inline auto rotated_inserts(std::string word) {
  return ranges::view::repeat_n(word, word.size()) |
         ranges::view::transform([n = 0](auto str) mutable {
           n++;
           return str.substr(1, n - 1) + std::string{str[0]} + str.substr(n);
         });
}

// In : pqr
// Out : [apqr,bpqr,cpqr ... zpqr]
inline auto pad_words(std::string word) {
  return ranges::view::zip_with(
      [](auto l, auto w) { return l + w; },
      ranges::view::concat(ranges::view::closed_iota('a', 'z'),
                           ranges::view::closed_iota('A', 'Z'),
                           ranges::view::closed_iota('0', '9'),
                           ranges::view::single('-')),
      ranges::view::single(word) | ranges::view::cycle);
}

// In: hi
// Out : [ahi,hai,hia,bhi ... hiz]
inline auto inserts(std::string word) {
  return pad_words(word) | ranges::view::transform([](auto padded_word) {
           return rotated_inserts(padded_word);
         }) |
         ranges::view::join;
}

// In : xabc
// Out : [xbc,axc,abx]
inline auto rotated_changes(std::string word) {
  return ranges::view::repeat_n(word, word.size() - 1) |
         ranges::view::transform([n = 0](auto str) mutable {
           n++;
           return str.substr(1, n - 1) + std::string{str[0]} +
                  str.substr(n + 1);
         });
}

// In: hi
// Out : [ai,ha,bi ... hz]
inline auto changes(std::string word) {
  return pad_words(word) | ranges::view::transform([](auto padded_word) {
           return rotated_changes(padded_word);
         }) |
         ranges::view::join;
}

inline auto all_edits() {
  return ranges::view::transform([](auto word) {
           return ranges::view::concat(deletes(word), changes(word),
                                       inserts(word));
         }) |
         ranges::view::join;
}

inline auto missing_module_instance_error(life::ModuleInstancePair mip) {
  auto &true_mod = mip.first;
  auto &attempted_inst = mip.second;
  std::cout << "Error: Non-existent <Module>::Instance -- \033[31m<" << true_mod
            << ">::" << attempted_inst << "\033[0m\n";
  for (auto &type_name_config_pair : life::all_configs) {
    auto &[mod, inst] = type_name_config_pair.first;
    if (mod == true_mod &&
        ranges::any_of(ranges::view::single(inst) | all_edits() |
                           all_edits(),
                       [=](auto i) { return i == attempted_inst; }))
      std::cout << "Did you mean \033[32m'" << inst << "'\033[0m?\n";
  }
  std::exit(1);
}


inline life::configuration true_parameters(life::ModuleInstancePair mip) {
  auto real_con_it = life::all_configs.find(mip);
  if (life::all_configs.end() == real_con_it)
	 missing_module_instance_error(mip); 
  return real_con_it->second;
}

inline void config_mismatch_error(std::string key, life::ModuleInstancePair mip) {

  std::cout << "Error: Configuration mismatch -- \033[33m<" << mip.first
            << ">::" << mip.second
            << "\033[0m does not have parameter named \033[31m'" << key
            << "'\033[0m\n";
  auto con = true_parameters(mip);
  for (auto it : con.items())
    if (ranges::any_of(ranges::view::single(key) | all_edits() |
                           all_edits(),
                       [=](auto s) { return s == it.key(); }))
      std::cout << "Did you mean \033[32m'" << it.key() << "'\033[0m?\n";

  std::exit(1);
}

inline void type_mismatch_error(std::string name, std::string real, std::string fake,
                         life::ModuleInstancePair mip) {
  std::cout << "Error: Type mismatch -- \033[33m<" << mip.first
            << ">::" << mip.second << "'" << name << "'\033[0m must have type \033[32m'"
            << real << "'\033[0m but has type \033[31m'" << fake
            << "'\033[0m\n";
  std::exit(1);
}

inline life::configuration true_object(life::ModuleInstancePair mip,
                                life::configuration con) {

  auto real_con = true_parameters(mip);

  for (auto it = con.begin(); it != con.end(); it++) {
    auto rit = real_con.find(it.key());
    if (rit == real_con.end())
      config_mismatch_error(it.key(), mip);

    if (rit->type_name() != it->type_name())
      type_mismatch_error(it.key(), rit->type_name(), it->type_name(), mip);

    rit.value() = it->type_name() == std::string{"array"}
                      ? life::configuration::array(
                            {it.value()[0],
                             true_object({std::string{rit.value()[0]}.substr(5),
                                          it.value()[0]},
                                         it.value()[1])})
                      : it.value();
  }
  return real_con;
}

}
} // namespace life
