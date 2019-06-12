#include <fstream>
#include <iostream>
#include "yaml-cpp/yaml.h"
#include "yavl.h"

using namespace std;

int main(int argc, char **argv)
{
  if(argc != 3) {
    std::cerr << "should have 2 arguments grammer file(schema) and file to be checked (argc = " << argc << " )\n";
    return -1;
  }
  // argv[1] grammar file
  // argv[2] file to be checked

  YAML::Node gr;
  try {
    gr = YAML::LoadFile(argv[2]);
  } catch(const YAML::Exception& e) {
    std::cerr << "Error reading grammar: " << e.what() << "\n";
    return 1;
  }

  YAML::Node doc;
  try {
    doc = YAML::LoadFile(argv[1]);
  } catch(const YAML::Exception& e) {
    std::cerr << "Error reading document: " << e.what() << "\n";
    return 2;
  }

  YAVL::Validator yavl(gr, doc);
  bool ok = yavl.validate();
  if (!ok) {
    cout << "ERRORS FOUND: " << endl << endl;
    cout << yavl.get_errors();
  }
  return !ok;
}


