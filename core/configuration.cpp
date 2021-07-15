

#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>

#include "configuration.hpp"
#include "utilities/term_colours.hpp"
#include "utilities/utilities.hpp"

namespace ded
{
namespace config_manager
{

void
    showConfig(std::ostream &out, std::string type_and_name)
{
  auto colon = type_and_name.find(':');
  if (colon == std::string::npos)
  {
    std::cout << "unrecognized <component-type>:<component-name> syntax";
    throw ConfigError{};
  }

  auto type = type_and_name.substr(0, colon);
  auto name = type_and_name.substr(colon + 1);

  if (type == "process")
  {
    if (!isProcessBlock(name))
    {
      std::cout << name + " is not a component of type " + type;
      throw ConfigError{};
    }
    out << ALL_PROCESS_SPECS[name].prettyPrint();
  }
  else if (type == "substrate")
  {
    if (!isSubstrateBlock(name))
    {
      std::cout << name + " is not a component of type " + type;
      throw ConfigError{};
    }
    out << ALL_SUBSTRATE_SPECS[name].prettyPrint();
  }
  else if (type == "population")
  {
    if (!isPopulationBlock(name))
    {
      std::cout << name + " is not a component of type " + type;
      throw ConfigError{};
    }
    out << ALL_POPULATION_SPECS[name].prettyPrint();
  }
  else if (type == "converter")
  {
    if (!isConverterBlock(name))
    {
      std::cout << name + " is not a component of type " + type;
      throw ConfigError{};
    }
    out << ALL_CONVERTER_SPECS[name].prettyPrint();
  }
  else if (type == "encoding")
  {
    if (!isEncodingBlock(name))
    {
      std::cout << name + " is not a component of type " + type;
      throw ConfigError{};
    }
    out << ALL_ENCODING_SPECS[name].prettyPrint();
  }
  else
  {
    std::cout << " unknown component type " + type;
    throw ConfigError{};
  }

  return;
}

void
    listAllConfigs(std::ostream &out)
{
  out << "substrate:\n";
  for (auto n_spec : ALL_SUBSTRATE_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "process:\n";
  for (auto n_spec : ALL_PROCESS_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "population:\n";
  for (auto n_spec : ALL_POPULATION_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "encoding:\n";
  for (auto n_spec : ALL_ENCODING_SPECS)
    out << "    " << n_spec.first << "\n";

  out << "converter:\n";
  for (auto n_spec : ALL_CONVERTER_SPECS)
    out << "    " << n_spec.first << "\n";

  return;
}

void
    saveAllConfigs()
{
  for (auto n_es : ALL_SUBSTRATE_SPECS)
    std::cout << "\n" << n_es.second.prettyPrint();
  for (auto n_es : ALL_PROCESS_SPECS)
    std::cout << "\n" << n_es.second.prettyPrint();
  for (auto n_es : ALL_POPULATION_SPECS)
    std::cout << "\n" << n_es.second.prettyPrint();
  for (auto n_es : ALL_ENCODING_SPECS)
    std::cout << "\n" << n_es.second.prettyPrint();
  for (auto n_es : ALL_CONVERTER_SPECS)
    std::cout << "\n" << n_es.second.prettyPrint();
  return;
}

bool
    isSubstrateBlock(std::string name)
{
  return ALL_SUBSTRATE_SPECS.find(name) != ALL_SUBSTRATE_SPECS.end();
}

bool
    isEncodingBlock(std::string name)
{
  return ALL_ENCODING_SPECS.find(name) != ALL_ENCODING_SPECS.end();
}

bool
    isProcessBlock(std::string name)
{
  return ALL_PROCESS_SPECS.find(name) != ALL_PROCESS_SPECS.end();
}

bool
    isPopulationBlock(std::string name)
{
  return ALL_POPULATION_SPECS.find(name) != ALL_POPULATION_SPECS.end();
}

bool
    isConverterBlock(std::string name)
{
  return ALL_CONVERTER_SPECS.find(name) != ALL_CONVERTER_SPECS.end();
}

std::vector<std::string>
    allSubstrateNames()
{
  return ALL_SUBSTRATE_SPECS | rv::keys | rs::to<std::vector<std::string>>;
}

std::vector<std::string>
    allProcessNames()
{
  return ALL_PROCESS_SPECS | rv::keys | rs::to<std::vector<std::string>>;
}

std::vector<std::string>
    allPopulationNames()
{
  return ALL_POPULATION_SPECS | rv::keys | rs::to<std::vector<std::string>>;
}

std::vector<std::string>
    allEncodingNames()
{
  return ALL_ENCODING_SPECS | rv::keys | rs::to<std::vector<std::string>>;
}

std::vector<std::string>
    allConverterNames()
{
  return ALL_CONVERTER_SPECS | rv::keys | rs::to<std::vector<std::string>>;
}

std::vector<std::string>
    allComponentNames()
{
  auto a = allSubstrateNames();
  auto b = allProcessNames();
  auto c = allPopulationNames();
  auto d = allEncodingNames();
  auto e = allConverterNames();
  return rv::concat(a, b, c, d, e) | rs::to<std::vector<std::string>>;
}

}   // namespace config_manager
}   // namespace ded
