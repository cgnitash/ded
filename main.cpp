

#include"components.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <regex>

template <typename L, typename M>
void save(std::ofstream &f, std::string_view component, L component_lister,
          M component_maker) {

  for (auto &comp_name : component_lister()) {
    auto comp  = component_maker(comp_name);
    auto con = comp.publish_configuration();

    for (auto &[name, vm] : con) {
      auto &[val, message] = vm;
      f << "<" << component << ">" << comp_name << "::" << name << " = " << val
        << "  #" << message << "\n";
    }
  }
}
void save_configs() {

  std::ofstream file("configurations.cfg");

  save(file, "experiment", life::experiment_list, life::make_experiment);
  save(file, "entity", life::entity_list, life::make_entity);
  save(file, "selector", life::selector_list, life::make_selector);
  file.close();
}

std::pair<std::string, life::configuration>
load_user_experiment(std::string file_name) {
  std::ifstream ef(file_name);
  std::string exp_name;
  life::configuration con;
  std::regex re(R"(^\s*(\w+)\s*>\s*$)");
  std::regex rv(R"(^\s*(\w+)\s*(\w+)\s*$)");
  std::regex rx(R"(^\s*(selector|entity)\s*(\w+)\s*>\s*$)");
  for (std::string s; std::getline(ef, s);) {
    std::smatch m;
    if (std::regex_match(s, m, re)) {
      exp_name = m[1].str();
      life::experiment e(life::make_experiment(exp_name));
      con = e.publish_configuration();
    }
    if (std::regex_match(s, m, rv)) {
      con[m[1]] = {m[2], ""};
    }
  }
	return {exp_name,con};
}
//con["generations"] = {"10",""};
int main(int argc, char **argv) {
  if (argc == 2 && std::string(argv[1]) == "-s") {
    save_configs();
  }

  if (argc == 3 && std::string(argv[1]) == "-s") {
    auto [exp_name, con] = load_user_experiment(argv[2]);
    auto n = life::configure_experiment(exp_name, con);
    n.run();
  }
}
