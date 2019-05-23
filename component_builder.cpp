
#include "core/term_colours.h"

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
    if (std::regex_match(clean_line, spaces)) continue;
    std::smatch m;
    if (std::regex_match(clean_line, m, r))
      build_options[m[1]].push_back(m[2]);
    else
    {
      std::cout << term_colours::red_fg
                << "<ded-core>Error:" << term_colours::reset
                << " unrecognised line in components file \""
                << term_colours::green_fg << fname << "\""
                << term_colours::reset << line << "\n";
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
  makefile << "\n# Auto generated by component_builder.cpp"
              "\n\nflags = "
           << args
           << " -Wall -std=c++17 -O3 "
              "\n\ndebugflags = "
           << args
           << " -Wall -std=c++17 -O0 -ggdb3 "
              "\n\nasanflags = "
           << args
           << " -Wall -std=c++17 -O3 -ggdb3 "
              "-fsanitize=address -static-libasan "
              "\n\nall : ded \n\n";

  makefile
      << "\ncore_headers = components.h core/environment.h "
         "core/entity.h core/population.h core/encoding.h "
         "core/configuration.h	"
         "core/signal.h core/utilities.h core/qst_parser.h";

  for (auto flags : { "", "debug", "asan" })
  {
    makefile << "\n\n"
             << flags << "components = " << flags << "obj_files/main.o "
             << flags << "obj_files/components.o " << flags
             << "obj_files/qst_parser.o " << flags
             << "obj_files/term_colours.o " << flags << "obj_files/util_csv.o "
             << flags << "obj_files/util_csvreader.o ";

    for (auto &[type, names] : build_options)
      for (auto &name : names)
        makefile << flags << "obj_files/" << type << "__" << name << ".o ";

    makefile << "\n\n"
             << flags << "ded : $(" << flags
             << "components)"
                "\n\t$("
             << flags << "flags) $(" << flags << "components) -lstdc++fs -o "
             << flags
             << "ded"
                "\n\n"
             << flags
             << "obj_files/main.o : main.cpp"
                "\n\t$("
             << flags << "flags) -c main.cpp -o " << flags
             << "obj_files/main.o"
                "\n\n"
             << flags
             << "obj_files/qst_parser.o : core/qst_parser.cpp"
                "\n\t$("
             << flags << "flags) -c core/qst_parser.cpp -o " << flags
             << "obj_files/qst_parser.o"
                "\n\n"
             << flags
             << "obj_files/term_colours.o : core/term_colours.cpp"
                "\n\t$("
             << flags << "flags) -c core/term_colours.cpp -o " << flags
             << "obj_files/term_colours.o"
                "\n\n"
             << flags
             << "obj_files/util_csv.o : core/csv/CSV.cpp"
                "\n\t$("
             << flags << "flags) -c core/csv/CSV.cpp -o " << flags
             << "obj_files/util_csv.o"
                "\n\n"
             << flags
             << "obj_files/util_csvreader.o : core/csv/CSVReader.cpp"
                "\n\t$("
             << flags
             << "flags) -c core/csv/CSVReader.cpp -o "
                ""
             << flags
             << "obj_files/util_csvreader.o"
                "\n\n"
             << flags
             << "obj_files/components.o : components.cpp "
                "$(core_headers) ";

    for (auto &[type, names] : build_options)
      for (auto &name : names)
        makefile << "user/" << type << "/" << name << ".h ";

    makefile << "\n\t$(" << flags << "flags) -c components.cpp -o " << flags
             << "obj_files/components.o\n\n";

    for (auto &[type, names] : build_options)
      for (auto &name : names)
        makefile << "" << flags << "obj_files/" << type << "__" << name
                 << ".o : user/" << type << "/" << name << ".cpp user/" << type
                 << "/" << name
                 << ".h "
                    "$(core_headers)\n"
                    "\t$("
                 << flags << "flags) -c user/" << type << "/" << name
                 << ".cpp -o " << flags << "obj_files/" << type << "__" << name
                 << ".o \n\n";

    makefile << "clean" << flags << " : \n\trm  " << flags << "obj_files/*.o "
             << flags << "ded\n\n";
  }
}

int
    main(int argc, char **argv)
{

  auto bo = get_build_options("components.cfg");
  generate_components("components.cpp", bo);
  generate_makefile("makefile", bo, argc > 1 ? argv[1] : "g++ ");
}
