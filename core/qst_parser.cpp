
#include "qst_parser.h"

// Input:
// layout:  "this is (0) (2)."
// varied:  {"a", "the"}
// 			{"fat", "thin"}
// 			{"(1) dog", "(1) cat"}
//
// Output:	this is a fat dog.
//			this is the fat dog.
//			this is a thin dog.
//			this is the thin dog.
//			this is a fat cat.
//			this is the fat cat.
//			this is a thin cat.
//			this is the thin cat.
std::vector<std::string>
qst_parser::expand_layout(std::string layout,
                          std::vector<std::vector<std::string>> varied) {
  std::vector<std::string> all_layouts;
  all_layouts.push_back(layout);
  return ranges::accumulate(
      ranges::view::reverse(varied), all_layouts,
      [count = varied.size() - 1](auto all_layouts, auto vary) mutable {
        std::regex r{"\\(" + std::to_string(count) + "\\)"};
        std::vector<std::string> current_layouts;
        ranges::transform(
            ranges::view::cartesian_product(all_layouts, vary),
            ranges::back_inserter(current_layouts), [r](const auto &t) {
              return std::regex_replace(std::get<0>(t), r, std::get<1>(t));
            });
        count--;
        return current_layouts;
      });
}

std::vector<std::tuple<std::string, std::string, std::string>>
qst_parser::parse_qst(std::string file_name) {

  std::vector<std::tuple<std::string, std::string, std::string>> all_exps;

  std::ifstream ifs(file_name);
  if (!ifs.is_open()) {
    std::cout << "Error: qst file \"" << file_name << "\" does not exist\n";
    std::exit(1);
  }

  std::map<std::string, std::string> all_variables;
  struct component_spec {
    std::string variable_or_comp_name, comp, params, pres, posts, in_sigs,
        out_sigs;
  };
  std::vector<component_spec> component_stack;
  std::vector<std::vector<std::string>> varied;
  std::vector<std::string> varied_labels;
  std::string line;
  std::smatch m;
  for (auto line_num{1}; std::getline(ifs, line); line_num++) {

    line = std::regex_replace(line, comments, "");

    if (line.empty())
      continue;

    if (std::regex_match(line, m, new_variable)) {
      if (!component_stack.empty()) {
        std::cout << "qst<syntax>error: new user variable cannot be nested "
                     "within "
                     "other components! line "
                  << line_num << "\n";
        std::exit(1);
      }
      if (m[3].str().empty()) {
        all_variables[m[1].str()] = "[\"" + m[2].str() +
                                    "\",{\"parameters\":null,\"pre-tags\":"
                                    "null,\"post-tags\":null}]";
      } else {
        component_stack.push_back({m[1].str(), m[2].str(), "", "", "", "", ""});
      }
      continue;
    }

    if (std::regex_match(line, m, new_refactored_variable)) {
      if (!component_stack.empty()) {
        std::cout
            << "qst<syntax>error: new-refactored-variable cannot be nested "
               "within "
               "other components! line "
            << line_num << "\n";
        std::exit(1);
      }

      auto name = m[2].str();
      auto variable = all_variables.find(name);
      if (variable == all_variables.end()) {
        std::cout << "error: refactored-variable " << name
                  << " not found! line " << line_num << "\n";
        std::exit(1);
      }

      all_variables[m[1].str()] = variable->second;

      continue;
    }

    if (std::regex_match(line, m, new_varied_variable)) {
      if (!component_stack.empty()) {
        std::cout << "qst<syntax>error: new-varied-variable cannot be nested "
                     "within "
                     "other components! line "
                  << line_num << "\n";
        std::exit(1);
      }

      std::vector<std::string> varied_entry;
      auto all_varied = m[2].str();
      auto varied_names = ranges::action::split(all_varied, ' ');
      varied_names.erase(
          ranges::remove_if(varied_names, [](auto s) { return s.empty(); }),
          ranges::end(varied_names));
      for (auto varied_name : varied_names) {
        varied_labels.push_back(m[1].str() + " = " + varied_name);
        if (varied_name[0] != '!' && varied_name[0] != '$') {

          std::cout << "error: varied-variable " << varied_name
                    << " must be a component or variable! line " << line_num
                    << "\n";
          std::exit(1);
        }
        if (varied_name[0] == '!') {
          auto variable = all_variables.find(varied_name.substr(1));
          if (variable == all_variables.end()) {
            std::cout << "error: varied-refactored-variable "
                      << varied_name.substr(1) << " not found! line "
                      << line_num << "\n";
            std::exit(1);
          }
          varied_name = variable->second + ",";
        } else {
          varied_name = varied_name.substr(1);
        }
        varied_entry.push_back(varied_name);
      }

      all_variables[m[1].str()] = "(" + std::to_string(varied.size()) + ")";
      varied.push_back(varied_entry);

      continue;
    }

    if (std::regex_match(line, m, nested_parameter)) {
      if (component_stack.empty()) {
        std::cout << "qst<syntax>error: parameter must be nested "
                     "within "
                     "other components! line "
                  << line_num << "\n";
        std::exit(1);
      }
      if (m[3].str().empty()) {
        component_stack.back().params += "\"" + m[1].str() + "\":[\"" +
                                         m[2].str() +
                                         "\",{\"parameters\":null,\"pre-tags\":"
                                         "null,\"post-tags\":null}],";
      } else {
        component_stack.push_back({m[1].str(), m[2].str(), "", "", "", "", ""});
      }
      continue;
    }

    if (std::regex_match(line, m, nested_refactored_parameter)) {
      if (component_stack.empty()) {
        std::cout << "qst<syntax>error: refactored-parameter must be nested "
                     "within "
                     "other components! line "
                  << line_num << "\n";
        std::exit(1);
      }

      auto name = m[2].str();
      auto variable = all_variables.find(name);
      if (variable == all_variables.end()) {
        std::cout << "error: refactored-variable " << name
                  << " not found! line " << line_num << "\n";
        std::exit(1);
      }
      component_stack.back().params +=
          "\"" + m[1].str() + "\":" + variable->second + ",";

      continue;
    }

    if (std::regex_match(line, m, nested_varied_parameter)) {
      if (component_stack.empty()) {
        std::cout << "qst<syntax>error: varied-parameter must be nested "
                     "within "
                     "other components! line "
                  << line_num << "\n";
        std::exit(1);
      }

      std::vector<std::string> varied_entry;
      auto all_varied = m[2].str();
      auto varied_names = ranges::action::split(all_varied, ' ');
      varied_names.erase(
          ranges::remove_if(varied_names, [](auto s) { return s.empty(); }),
          ranges::end(varied_names));
      for (auto &varied_name : varied_names)
        varied_labels.push_back(m[1].str() + " = " + varied_name);
      auto is_not_primitive = [](auto s) { return s[0] == '$' || s[0] == '!'; };
      auto compvars = ranges::all_of(varied_names, is_not_primitive);
      auto primitives = ranges::none_of(varied_names, is_not_primitive);
      if (!(compvars || primitives)) {
        std::cout << "error: varied-parameters must all be  "
                     "components/variables, or they must all be primitive "
                     "values! line "
                  << line_num << "\n";
        std::exit(1);
      }
      if (compvars) {

        for (auto varied_name : varied_names) {
          if (varied_name[0] == '!') {
            auto variable = all_variables.find(varied_name.substr(1));
            if (variable == all_variables.end()) {
              std::cout << "error: varied-refactored-variable "
                        << varied_name.substr(1) << " not found! line "
                        << line_num << "\n";
              std::exit(1);
            }
            varied_name = "\"" + m[1].str() + "\":" + variable->second + ",";
          } else { // if varied_name[0] == '$'
            varied_name = "\"" + m[1].str() + "\":[\"" + varied_name.substr(1) +
                          "\",{\"parameters\":null,\"pre-tags\":"
                          "null,\"post-tags\":null}],";
          }
          varied_entry.push_back(varied_name);
        }
      } else { // if primitives
        for (auto varied_name : varied_names) {
          varied_name = "\"" + m[1].str() + "\":" + varied_name + ",";
          varied_entry.push_back(varied_name);
        }
      }

      component_stack.back().params +=
          "(" + std::to_string(varied.size()) + ")";
      varied.push_back(varied_entry);

      continue;
    }

    if (std::regex_match(line, m, parameter)) {
      component_stack.back().params +=
          "\"" + m[1].str() + "\":" + m[2].str() + ",";
      continue;
    }

    if (std::regex_match(line, m, pre_tag)) {
      component_stack.back().pres +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, post_tag)) {
      component_stack.back().posts +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, in_signal_tag)) {
      component_stack.back().in_sigs +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, out_signal_tag)) {
      component_stack.back().out_sigs +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, close_brace)) {
      if (component_stack.empty()) {
        std::cout << "qst<syntax>error: dangling closing brace! line "
                  << line_num << "\n";
        std::exit(1);
      }
      auto current = component_stack.back();
      component_stack.pop_back();
      auto expanded_component =
          "[\"" + current.comp + "\",{\"parameters\":{" + current.params +
          "},\"pre-tags\":{" + current.pres + "},\"post-tags\":{" +
          current.posts + "},\"org-inputs\":{" + current.in_sigs +
          "},\"org-outputs\":{" + current.out_sigs + "}}]";
      if (component_stack.empty()) {
        all_variables[current.variable_or_comp_name] = expanded_component;
      } else {
        component_stack.back().params += "\"" + current.variable_or_comp_name +
                                         "\":" + expanded_component + ",";
      }
      continue;
    }

    std::cout << "qst<syntax>error: unable to parse! line " << line_num << "\n"
              << line << "\n";
    std::exit(1);
  }

  if (!component_stack.empty()) {
    std::cout << "qst<syntax>error: braces need to be added\n";
    std::exit(1);
  }

  auto pop_env_layout =
      std::regex_replace(all_variables["P"], spurious_commas, "$1") + '@' +
      std::regex_replace(all_variables["E"], spurious_commas, "$1");

  auto all_layouts = varied.empty() ? std::vector{pop_env_layout}
                                    : expand_layout(pop_env_layout, varied);

  varied_labels = varied_labels.empty() ? std::vector<std::string>{"__RAW__"}
                                        : varied_labels;

  std::vector<std::pair<std::string, std::string>> all_exp_cons =
      ranges::view::zip(all_layouts, varied_labels);

  all_exp_cons |=
      ranges::action::sort(std::less<std::string>{},
                           &std::pair<std::string, std::string>::first) |
      ranges::action::unique(std::equal_to<std::string>{},
                             &std::pair<std::string, std::string>::first);

  for (auto &[exp, label] : all_exp_cons) {

    auto marker = exp.find('@');

    all_exps.push_back(std::make_tuple(
        std::regex_replace(exp.substr(0, marker), spurious_commas, "$1"),
        std::regex_replace(exp.substr(marker + 1), spurious_commas, "$1"),
        label));
  }

  return all_exps;
}

