
#include "qst_parser.h"
#include "term_colours.h"

// Input:
// layout:  "this is (0) (2)."
// varied:  {"a", "the"}
// 		    {"fat", "thin"}
// 	        {"(1) dog", "(1) cat"}
//
// Output:	this is a fat dog.
//	        this is the fat dog.
//          this is a thin dog.
//          this is the thin dog.
//          this is a fat cat.
//          this is the fat cat.
//          this is a thin cat.
//          this is the thin cat.
std::vector<std::string>
    qst_parser::expand_layout(std::string                           layout,
                              std::vector<std::vector<std::string>> varied)
{
  return ranges::accumulate(
      // iterate through indices of varied in reverse
      ranges::view::iota(0, varied.size()) | ranges::view::reverse,
      // accumulator begins with layout
      std::vector<std::string>{ layout },
      // to expand a single layout by index
      [&](const auto &all_layouts, auto index) -> std::vector<std::string> {
        // create the index-pattern to be substituted
        const std::regex r{ "\\(" + std::to_string(index) + "\\)" };
        // and replace all occurences of pattern with all varied substitutions
        return ranges::view::cartesian_product(all_layouts, varied[index]) |
               ranges::view::transform([&r](const auto &t) {
                 return std::regex_replace(std::get<0>(t), r, std::get<1>(t));
               });
      });
}

void
    qst_parser::check_no_redefinition(std::string name)
{
  if (all_variables.find(name) != all_variables.end())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " redefinition of variable '" << name << "' !line " << line_num
              << "\n";
    std::exit(1);
  }
}

void
    qst_parser::check_global_scope()
{
  if (!component_stack.empty())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " new user variable cannot be "
                 "nested "
                 "within other components! line "
              << line_num << "\n";
    std::exit(1);
  }
}

void
    qst_parser::parse_new_variable(std::smatch m)
{
  check_global_scope();

  if (m[3].str().empty())
  {
    check_no_redefinition(m[1].str());
    all_variables[m[1].str()] = "[\"" + m[2].str() +
                                "\",{\"parameters\":null,\"pre-tags\":"
                                "null,\"post-tags\":null}]";
  } else
  {
    component_stack.push_back({ m[1].str(), m[2].str(), "", "", "", "", "" });
  }
}

void
    qst_parser::parse_new_refactored_variable(std::smatch m)
{
  check_global_scope();

  auto name     = m[2].str();
  auto variable = all_variables.find(name);

  if (variable == all_variables.end())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " refactored-variable " << name << " not found! line "
              << line_num << "\n";
    std::exit(1);
  }

  check_no_redefinition(m[1].str());
  all_variables[m[1].str()] = variable->second;
}

void
    qst_parser::parse_new_varied_variable(std::smatch m)
{
  check_global_scope();

  std::vector<std::string> varied_entry;

  auto all_varied   = m[2].str();
  auto varied_names = ranges::action::split(all_varied, ' ');

  varied_names.erase(
      ranges::remove_if(varied_names, [](auto s) { return s.empty(); }),
      ranges::end(varied_names));

  varied_labels.push_back(varied_names |
                          ranges::view::transform([v = m[1].str()](auto n) {
                            return v + "=" + n;
                          }));

  for (auto varied_name : varied_names)
  {
    if (varied_name[0] != '!' && varied_name[0] != '$')
    {
      std::cout << "" << term_colours::red_fg
                << "qst<syntax>error:" << term_colours::reset
                << " varied-variable " << varied_name
                << " must be a component or variable! line " << line_num
                << "\n";
      std::exit(1);
    }
    if (varied_name[0] == '!')
    {
      auto variable = all_variables.find(varied_name.substr(1));
      if (variable == all_variables.end())
      {
        std::cout << "" << term_colours::red_fg
                  << "qst<syntax>error:" << term_colours::reset
                  << " varied-refactored-variable " << varied_name.substr(1)
                  << " not found! line " << line_num << "\n";
        std::exit(1);
      }
      varied_name = variable->second + ",";
    } else
    {
      varied_name = varied_name.substr(1);
    }
    varied_entry.push_back(varied_name);
  }

  check_no_redefinition(m[1].str());
  all_variables[m[1].str()] = "(" + std::to_string(varied.size()) + ")";
  varied.push_back(varied_entry);
}

void
    qst_parser::parse_nested_parameter(std::smatch m)
{
  if (component_stack.empty())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " parameter must be nested "
                 "within other components! line "
              << line_num << "\n";
    std::exit(1);
  }
  if (m[3].str().empty())
  {
    component_stack.back().params += "\"" + m[1].str() + "\":[\"" + m[2].str() +
                                     "\",{\"parameters\":null,\"pre-tags\":"
                                     "null,\"post-tags\":null}],";
  } else
  {
    component_stack.push_back({ m[1].str(), m[2].str(), "", "", "", "", "" });
  }
}

void
    qst_parser::parse_nested_refactored_parameter(std::smatch m)
{
  if (component_stack.empty())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " refactored-parameter must "
                 "be nested "
                 "within "
                 "other components! line "
              << line_num << "\n";
    std::exit(1);
  }

  auto name     = m[2].str();
  auto variable = all_variables.find(name);

  if (variable == all_variables.end())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " refactored-variable " << name << " not found! line "
              << line_num << "\n";
    std::exit(1);
  }

  component_stack.back().params +=
      "\"" + m[1].str() + "\":" + variable->second + ",";
}

void
    qst_parser::parse_nested_varied_parameter(std::smatch m)
{
  if (component_stack.empty())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " varied-parameter must be nested "
                 "within "
                 "other components! line "
              << line_num << "\n";
    std::exit(1);
  }

  std::vector<std::string> varied_entry;

  auto all_varied   = m[2].str();
  auto varied_names = ranges::action::split(all_varied, ' ');

  varied_names.erase(
      ranges::remove_if(varied_names, [](auto s) { return s.empty(); }),
      ranges::end(varied_names));

  varied_labels.push_back(varied_names |
                          ranges::view::transform([v = m[1].str()](auto n) {
                            return v + "=" + n;
                          }));

  auto is_not_primitive = [](auto s) { return s[0] == '$' || s[0] == '!'; };
  auto compvars         = ranges::all_of(varied_names, is_not_primitive);
  auto primitives       = ranges::none_of(varied_names, is_not_primitive);

  if (!(compvars || primitives))
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " varied-parameters must all be  "
                 "components/variables, or they must all be primitive "
                 "values! line "
              << line_num << "\n";
    std::exit(1);
  }

  if (compvars)
  {
    for (auto varied_name : varied_names)
    {
      if (varied_name[0] == '!')
      {
        auto variable = all_variables.find(varied_name.substr(1));
        if (variable == all_variables.end())
        {
          std::cout << "" << term_colours::red_fg
                    << "qst<syntax>error:" << term_colours::reset
                    << " varied-refactored-variable " << varied_name.substr(1)
                    << " not found! line " << line_num << "\n";
          std::exit(1);
        }
        varied_name = "\"" + m[1].str() + "\":" + variable->second + ",";
      } else
      {   // if varied_name[0] == '$'
        varied_name = "\"" + m[1].str() + "\":[\"" + varied_name.substr(1) +
                      "\",{\"parameters\":null,\"pre-tags\":"
                      "null,\"post-tags\":null}],";
      }
      varied_entry.push_back(varied_name);
    }
  } else
  {   // if primitives
    for (auto varied_name : varied_names)
    {
      varied_name = "\"" + m[1].str() + "\":" + varied_name + ",";
      varied_entry.push_back(varied_name);
    }
  }

  component_stack.back().params += "(" + std::to_string(varied.size()) + ")";
  varied.push_back(varied_entry);
}

void
    qst_parser::parse_closed_brace()
{
  if (component_stack.empty())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " dangling closing brace! line " << line_num << "\n";
    std::exit(1);
  }
  auto current = component_stack.back();
  component_stack.pop_back();
  auto expanded_component = "[\"" + current.comp + "\",{\"parameters\":{" +
                            current.params + "},\"pre-tags\":{" + current.pres +
                            "},\"post-tags\":{" + current.posts +
                            "},\"input-tags\":{" + current.in_sigs +
                            "},\"output-tags\":{" + current.out_sigs + "}}]";

  if (component_stack.empty())
  {
    check_no_redefinition(current.variable_or_comp_name);
    all_variables[current.variable_or_comp_name] = expanded_component;
  } else
  {
    component_stack.back().params +=
        "\"" + current.variable_or_comp_name + "\":" + expanded_component + ",";
  }
}

void
    qst_parser::cleanup()
{

  if (!component_stack.empty())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " braces need to be added\n";
    std::exit(1);
  }

  if (all_variables.find("P") == all_variables.end())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " qst script must have a "
                 "variable named 'P' for the "
                 "Population\n";
    std::exit(1);
  }

  if (all_variables.find("E") == all_variables.end())
  {
    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " qst script must have a "
                 "variable named 'E' for the "
                 "Environment\n";
    std::exit(1);
  }

  all_variables["E"] =
      "[\"pass_through\",{\"parameters\":{\"env\":" + all_variables["E"] +
      "},\"pre-tags\":{},\"post-tags\":{},\"input-tags\":{},\"output-tags\":{}}"
      "]";

  auto pop_env_layout =
      std::regex_replace(all_variables["P"], spurious_commas, "$1") + '@' +
      std::regex_replace(all_variables["E"], spurious_commas, "$1");

  auto all_layouts = varied.empty() ? std::vector{ pop_env_layout }
                                    : expand_layout(pop_env_layout, varied);

  auto marked_labels =
      varied_labels.empty()
          ? std::vector<std::string>{ "__RAW__" }
          : expand_layout(
                ranges::accumulate(ranges::view::iota(0u, varied_labels.size()),
                                   std::string{},
                                   [](auto s, auto i) {
                                     return s + "(" + std::to_string(i) + ") ";
                                   }),
                varied_labels);

  std::vector<std::pair<std::string, std::string>> all_exp_cons =
      ranges::view::zip(all_layouts, marked_labels);

  all_exp_cons |=
      ranges::action::sort(std::less<std::string>{},
                           &std::pair<std::string, std::string>::first) |
      ranges::action::unique(std::equal_to<std::string>{},
                             &std::pair<std::string, std::string>::first);

  for (auto &[exp, label] : all_exp_cons)
  {

    auto marker = exp.find('@');

    all_exps.push_back(std::make_tuple(
        std::regex_replace(exp.substr(0, marker), spurious_commas, "$1"),
        std::regex_replace(exp.substr(marker + 1), spurious_commas, "$1"),
        label));
  }
}

std::vector<std::tuple<std::string, std::string, std::string>>
    qst_parser::parse_qst(std::string file_name)
{

  std::ifstream ifs(file_name);
  if (!ifs.is_open())
  {
    std::cout << "Error: qst file \"" << file_name << "\" does not exist\n";
    std::exit(1);
  }

  std::string line;
  std::smatch m;
  for (; std::getline(ifs, line); line_num++)
  {

    line = std::regex_replace(line, comments, "");

    line = std::regex_replace(line, spurious_tabs, " ");

    if (line.empty() || ranges::all_of(line, [](auto c) { return c == ' '; }))
      continue;

    if (std::regex_match(line, m, new_variable))
    {
      parse_new_variable(m);
      continue;
    }

    if (std::regex_match(line, m, new_refactored_variable))
    {
      parse_new_refactored_variable(m);
      continue;
    }

    if (std::regex_match(line, m, new_varied_variable))
    {
      parse_new_varied_variable(m);
      continue;
    }

    if (std::regex_match(line, m, nested_parameter))
    {
      parse_nested_parameter(m);
      continue;
    }

    if (std::regex_match(line, m, nested_refactored_parameter))
    {
      parse_nested_refactored_parameter(m);
      continue;
    }

    if (std::regex_match(line, m, nested_varied_parameter))
    {
      parse_nested_varied_parameter(m);
      continue;
    }

    if (std::regex_match(line, m, parameter))
    {
      component_stack.back().params +=
          "\"" + m[1].str() + "\":" + m[2].str() + ",";
      continue;
    }

    if (std::regex_match(line, m, pre_tag))
    {
      component_stack.back().pres +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, post_tag))
    {
      component_stack.back().posts +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, in_signal_tag))
    {
      component_stack.back().in_sigs +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, m, out_signal_tag))
    {
      component_stack.back().out_sigs +=
          "\"" + m[1].str() + "\":\"" + m[2].str() + "\",";
      continue;
    }

    if (std::regex_match(line, close_brace))
    {
      parse_closed_brace();
      continue;
    }

    std::cout << "" << term_colours::red_fg
              << "qst<syntax>error:" << term_colours::reset
              << " unable to parse! line " << line_num << "\n"
              << line << "\n";
    std::exit(1);
  }

  cleanup();

  return all_exps;
}
