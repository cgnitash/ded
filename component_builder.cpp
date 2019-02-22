
#include "core/fmt/fmt/format.h"
#include "core/fmt/fmt/printf.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

using opts = std::map<std::string, std::vector<std::string>>;

opts
    get_build_options(const std::string &fname)
{

  opts          build_options;
  std::ifstream cfg(fname);
  std::regex    r(R"~~(^\s*(entity|environment|population)\s*:\s*(\w+)\s*$)~~");
  std::regex    comments(R"~~(#.*$)~~");
  std::regex    spaces(R"~~(^\s*$)~~");
  for (std::string line; std::getline(cfg, line);)
  {
    auto clean_line = std::regex_replace(line, comments, "");
    if (std::regex_match(clean_line,spaces))continue;
    std::smatch m;
    if (std::regex_match(clean_line, m, r))
      build_options[m[1]].push_back(m[2]);
    else
    {
      std::cout << "\033[31m<ded-core>Error:\033[0m unrecognised line in "
                   "components file \033[32m\""
                << fname << "\"\033[0m\n*"
                << line << "*\n";
      std::exit(1);
    }
  }
  return build_options;
}

void
    generate_components(const std::string &fname, opts build_options)
{

  std::ofstream header(fname);

  header << "\n// Auto generated by "
            "component_builder.cpp\n\n#include\"components.h\"\n\n";

  for (auto &[type, names] : build_options)
    for (auto &name : names)
      header << "#include\"user/" << type << "/" << name << ".h\"\n";

  header << "\n#include<string_view>\n\nnamespace life {\n\n";

  for (auto &[type, names] : build_options)
  {
    header << type << " make_" << type << "(configuration con) {\n";
    for (auto &name : names)
      header << "  if (con[0] == \"" << name << "\") {\n    auto e = " << type
             << "{" << name
             << "()};\n    e.configure(con[1]);\n    return e;\n  }\n";
    header << "  std::cout << \"unknown-" << type
           << ": \" << con[0];\n  exit(1);\n}\n\n";

    for (auto &name : names)
      header << "template<>\nstd::string auto_class_name_as_string<" << name
             << ">() { return \"" << name << "\"; }\n\n";
  }

  header << "void generate_all_configs() {\n";
  for (auto &[type, names] : build_options)
  {
    header << "  generate_config(\"" << type << "\",life::make_" << type
           << ", {";
    std::string name_list;
    for (auto &name : names) name_list += "\"" + name + "\",";
    name_list.pop_back();
    header << name_list << "});\n";
  }
  header << "}\n\n} // namespace life";
}

void
    generate_makefile(const std::string &fname,
                      opts               build_options,
                      std::string        args)
{
  std::ofstream makefile(fname);
  using namespace fmt::literals;
  makefile << "\n# Auto generated by component_builder.cpp"
              "\n\nflags = {0} -Wall -std=c++17 -O3 "
              "\n\ndebugflags = {0} -Wall -std=c++17 -O0 -ggdb3 "
              "\n\nasanflags = {0} -Wall -std=c++17 -O3 -ggdb3 "
              "-fsanitize=address -static-libasan "
              "\n\nall : ded \n\n"_format(args);

  makefile
      << "\ncore_headers = components.h core/experiment.h core/environment.h "
         "core/entity.h core/population.h core/encoding.h "
         "core/configuration.h	"
         "core/signal.h core/utilities.h ";

  for (auto &flags : { "", "debug", "asan" })
  {
    makefile << "\n\n{0}components = {0}obj_files/main.o "
                "{0}obj_files/components.o {0}obj_files/qst_parser.o "
                "{0}obj_files/util_csv.o {0}obj_files/util_csvreader.o "_format(
                    flags);

    for (auto &[type, names] : build_options)
      for (auto &name : names)
        makefile << "{0}obj_files/{1}__{2}.o "_format(flags, type, name);

    makefile
        << "\n\n{0}ded : $({0}components)"
           "\n\t$({0}flags) $({0}components) -lstdc++fs -o {0}ded"
           "\n\n{0}obj_files/main.o : main.cpp"
           "\n\t$({0}flags) -c main.cpp -o {0}obj_files/main.o"
           "\n\n{0}obj_files/qst_parser.o : core/qst_parser.cpp"
           "\n\t$({0}flags) -c core/qst_parser.cpp -o {0}obj_files/qst_parser.o"
           "\n\n{0}obj_files/util_csv.o : core/csv/CSV.cpp"
           "\n\t$({0}flags) -c core/csv/CSV.cpp -o {0}obj_files/util_csv.o"
           "\n\n{0}obj_files/util_csvreader.o : core/csv/CSVReader.cpp"
           "\n\t$({0}flags) -c core/csv/CSVReader.cpp -o "
           "{0}obj_files/util_csvreader.o"
           "\n\n{0}obj_files/components.o : components.cpp "
           "$(core_headers) "_format(flags);
    for (auto &[type, names] : build_options)
      for (auto &name : names) makefile << "user/{0}/{1}.h "_format(type, name);

    makefile
        << "\n\t$({0}flags) -c components.cpp -o {0}obj_files/components.o\n\n"_format(
               flags);

    for (auto &[type, names] : build_options)
      for (auto &name : names)
        makefile
            << "{0}obj_files/{1}__{2}.o : user/{1}/{2}.cpp user/{1}/{2}.h "
               "$(core_headers)\n"
               "\t$({0}flags) -c user/{1}/{2}.cpp -o {0}obj_files/{1}__{2}.o \n\n"_format(
                   flags, type, name);

    makefile << "clean{0} : \n\trm  {0}obj_files/*.o {0}ded\n\n"_format(flags);
  }
}

int
    main(int argc, char **argv)
{

  auto bo = get_build_options("components.cfg");
  generate_components("components.cpp", bo);
  generate_makefile("makefile", bo, argc > 1 ? argv[1] : "g++ ");
}
