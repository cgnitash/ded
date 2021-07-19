
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include "core/utilities/utilities.hpp"

using Options = std::map<std::string, std::vector<std::string>>;

Options
    getBuildOptions(std::string fname)
{

  Options       build_options;
  std::ifstream cfg(fname);
  if (!cfg.is_open())
  {
    std::cout << "Error: component file " << fname << " not found" << std::endl;
    std::exit(1);
  }
  std::regex r(
      R"~~(^\s*(substrate|process|population|converter)\s*:\s*(\w+)\s*$)~~");
  std::regex comments(R"~~(#.*$)~~");
  std::regex spaces(R"~~(^\s*$)~~");
  for (std::string line; std::getline(cfg, line);)
  {
    auto clean_line = std::regex_replace(line, comments, "");
    if (std::regex_match(clean_line, spaces))
      continue;
    std::smatch m;
    if (std::regex_match(clean_line, m, r))
      build_options[m[1]].push_back(m[2]);
    else
    {
      std::cout << "Error: unrecognised line in components file \"" << fname
                << "\"" << line << "\n";
      std::exit(1);
    }
  }
  return build_options;
}

void
    generateComponents(Options build_options)
{

  std::ofstream header("components.cpp");

  header << "\n// Auto generated by "
            "component_builder.cpp\n\n#include\"components.hpp\"\n\n";

  for (auto [type, names] : build_options)
    for (auto name : names)
      header << "#include\"user/" << type << "/" << name << "/" << name
             << ".hpp\"\n";

  header << "\n#include<string_view>\n\nnamespace ded {\n\n";

  for (auto const &[lower_case_type, names] : build_options)
  {
    auto type = lower_case_type;
    type[0]   = std::toupper(type[0]);
    header << "specs::" << type << "Spec default" << type
           << "Spec(std::string name) \n{\n";
    for (auto name : names)
      header << "  if (name == \"" << name
             << "\") \n  {\n    auto e = concepts::" << type << "{" << name
             << "()};\n    return e.publishConfiguration();\n  }\n";
    header << "  std::cout << \"unknown-" << type
           << ": \" << name;\n  throw specs::SpecError{};\n}\n\n";
  }

  for (auto const &[lower_case_type, names] : build_options)
  {
    auto type = lower_case_type;
    type[0]   = std::toupper(type[0]);
    header << "concepts::" << type << " make" << type << "(specs::" << type
           << "Spec spec) \n{\n";
    for (auto name : names)
      header << "  if (spec.name() == \"" << name
             << "\") \n  {\n    auto e = concepts::" << type << "{" << name
             << "()};\n    e.configure(spec);\n    return e;\n  }\n";
    header << "  std::cout << \"unknown-" << type
           << ": \" << spec.name();\n  throw specs::SpecError{};\n}\n\n";
  }

  for (auto name : rv::concat(build_options["substrate"],
                              build_options["process"],
                              build_options["converter"],
                              build_options["population"]))
    header << "template<>\nstd::string autoClassNameAsString<" << name
           << ">() \n{ return \"" << name << "\"; }\n\n";

  header << "void generateAllSpecs() {\n";
  for (auto const &[lower_case_type, names] : build_options)
  {
    auto type = lower_case_type;
    type[0]   = std::toupper(type[0]);
    header << "  generate" << type << "Spec({"
           << (names | rv::transform([](auto s) { return "\"" + s + "\""; }) |
               rv::intersperse(",") | ra::join)
           << "});\n";
  }
  // hard coded until encoding spec interface is provided
  header << "  ALL_ENCODING_SPECS[\"encoding\"] = "
            "ded::concepts::Encoding{}.publishConfiguration();\n}\n";

  // this goes soon
  header << R"~(
inline void
    generateSubstrateSpec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    try
    {
      auto c =  ded::defaultSubstrateSpec(comp_name);
      ded::makeSubstrate(c);
      ded::ALL_SUBSTRATE_SPECS[comp_name] = c;
    }
    catch (ded::specs::SpecError const &e)
    {
      std::cout << "Error in component Substrate/" << comp_name << "\n";
      throw e;
    }
}

inline void
    generateProcessSpec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    try
    {
      auto c =  ded::defaultProcessSpec(comp_name);
      ded::makeProcess(c);
      ded::ALL_PROCESS_SPECS[comp_name] = c;
    }
    catch (ded::specs::SpecError const &e)
    {
      std::cout << "Error in component Process/" << comp_name << "\n";
      throw e;
    }
}

inline void
    generatePopulationSpec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    try
    {
      auto c =  ded::defaultPopulationSpec(comp_name);
      ded::makePopulation(c);
      ded::ALL_POPULATION_SPECS[comp_name] = c;
    }
    catch (ded::specs::SpecError const &e)
    {
      std::cout << "Error in component Population/" << comp_name << "\n";
      throw e;
    }
}

inline void
    generateConverterSpec(std::initializer_list<std::string> component_list)
{
  for (auto comp_name : component_list)
    try
    {
      auto c =  ded::defaultConverterSpec(comp_name);
      ded::makeConverter(c);
      ded::ALL_CONVERTER_SPECS[comp_name] = c;
    }
    catch (ded::specs::SpecError const &e)
    {
      std::cout << "Error in component Converter/" << comp_name << "\n";
      throw e;
    }
}
)~";

  header << "\n} // namespace ded";
}

void
    generateMakefile(const std::string &fname,
                     Options            build_options,
                     std::string        args)
{
  auto asHeader = [](auto s) { return s + ".hpp "; };
  auto asSource = [](auto s) { return s + ".cpp "; };
  auto asObject = [](auto s) {
    std::replace(s.begin(), s.end(), '/', '_');
    return "obj_files/" + s + ".o ";
  };

  std::vector<std::string> core_files = { "core/configuration",
                                          "core/experiments",
                                          "core/analysis",
                                          "core/utilities/csv/csv",
                                          "core/utilities/csv/csv_reader",
                                          "core/utilities/term_colours",
                                          "core/utilities/utilities",
                                          "core/language/token",
                                          "core/language/lexer",
                                          "core/language/parser",
                                          "core/specs/configuration_primitive",
                                          "core/specs/signal_spec",
                                          "core/specs/population_spec",
                                          "core/specs/substrate_spec",
                                          "core/specs/process_spec",
                                          "core/specs/converter_spec",
                                          "core/specs/encoding_spec",
                                          "core/concepts/substrate",
                                          "core/concepts/process",
                                          "core/concepts/population",
                                          "core/concepts/converter",
                                          "core/concepts/signal",
                                          "core/concepts/encoding" };

  std::vector<std::string> user_files;
  for (auto const &[type, names] : build_options)
    for (auto &name : names)
      user_files.push_back("user/" + type + "/" + name + "/" + name);

  std::ofstream makefile(fname);

  makefile << "\n# Auto generated by component_builder.cpp\n\nflags = " << args;

  makefile << "\n\nheaders = components.hpp "
           << (core_files | rv::transform(asHeader) | ra::join);

  makefile << "\n\ncomponents = obj_files/main.o obj_files/components.o "
           << (core_files | rv::transform(asObject) | ra::join)
           << (user_files | rv::transform(asObject) | ra::join);

  makefile << "\n\nded : $(components)"
              "\n\t$(flags) $(components) -lstdc++fs -o ded";

  makefile << "\n\nobj_files/main.o : main.cpp "
              "\n\t$(flags) -c main.cpp -o obj_files/main.o\n"
           << (core_files | rv::transform([&](auto file) {
                 return "\n" + asObject(file) + ": " + asSource(file) +
                        "\n\t$(flags) -c " + asSource(file) + " -o " +
                        asObject(file) + "\n";
               }) |
               ra::join)
           << "\nobj_files/components.o : components.cpp $(headers) "
           << (user_files | rv::transform(asHeader) | ra::join)
           << "\n\t$(flags) -c components.cpp -o "
           << "obj_files/components.o\n"
           << (user_files | rv::transform([&](auto file) {
                 return "\n" + asObject(file) + ": " + asSource(file) +
                        asHeader(file) + " $(headers) \n\t$(flags) -c " +
                        asSource(file) + "-o " + asObject(file) + "\n";
               }) |
               ra::join);

  makefile << "\n\nclean : "
           << "\n\trm -f obj_files/*.o"
           << "\n\trm -f ded\n\n";
}

int
    main(int argc, char **argv)
{

  auto bo = getBuildOptions("components.cfg");
  generateComponents(bo);
  generateMakefile(
      "makefile", bo, argc > 1 ? argv[1] : "g++ -Wall -std=c++17 -O3 ");
  // " -Wall -std=c++17 -O3 "
  // " -Wall -std=c++17 -O0 -ggdb3 "
  // " -Wall -std=c++17 -O3 -ggdb3 -fsanitize=address -static-libasan "
}
