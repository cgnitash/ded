#include <fstream>
#include <iostream>
#include <regex>

class OrgDot {

  struct Org {
    // long id;
    long recorded_at;
    long on_lod;
    std::vector<long> children;
  };
  std::map<long, Org> orgs;
  const std::regex r{R"(^([^,]+),([^,]+),([^,]+),([^,]+)$)"};

public:
  OrgDot() { orgs[0] = {0, 1, {}}; }

  void update_orgs(std::smatch m) {
    auto id = std::stol(m[1].str());
    orgs[id].recorded_at = std::stol(m[2].str());
    orgs[id].on_lod = std::stol(m[3].str());

    orgs[std::stol(m[4].str())].children.push_back(id);
  }

  void load_from(std::string name) {
    std::string line;
    std::ifstream file(name);
    std::getline(file, line);
    std::smatch m;
    while (std::getline(file, line)) {
      std::regex_match(line, m, r);
      update_orgs(m);
    }
  }

  void write_dot(std::string name) {
    std::ofstream dot_file(name);
    dot_file << "digraph {\n";
    for (auto &[org_id, org] : orgs) {
      dot_file << org_id << (org.on_lod ? " [color=red]" : " [color=blue]")
               << ";\n";
      dot_file << org_id << " -> { ";
      for (auto &child : org.children)
        if (!orgs[child].on_lod)
          dot_file << child << " ";
      dot_file << "} [color=blue];\n";
      for (auto &child : org.children)
        if (orgs[child].on_lod)
          dot_file << org_id << " -> " << child << "[color=red];\n";
    }
    dot_file << "}\n";
  }
};

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "error: no lineage file specified" << std::endl;
    exit(1);
  }
  OrgDot O;
  std::string dir = argv[1];
  std::regex path_sym("/");
	auto ndir = std::regex_replace(dir,path_sym,"_");
  O.load_from(dir + std::string{"/lineage.csv"});
  O.load_from(dir + std::string{"/unpruned.csv"});
  O.write_dot(ndir + "lod.dot");
}
