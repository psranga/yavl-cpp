#include <fstream>
#include <iostream>
#include "yaml.h"
#include "yavl.h"

using namespace std;
using namespace YAVL;

int main(int argc, char **argv)
{
  std::ifstream grin; // grammar file
  grin.open(argv[1]);
  
  std::ifstream yin; // file to be checked
  yin.open(argv[2]);
  
  YAML::Node gr;
  try {
    YAML::Parser parser(grin);
    parser.GetNextDocument(gr);
  } catch(const YAML::Exception& e) {
    std::cerr << "Error reading grammar: " << e.what() << "\n";
    return 1;
  }

  YAML::Node doc;
  try {
    YAML::Parser parser(yin);
    parser.GetNextDocument(doc);
  } catch(const YAML::Exception& e) {
    std::cerr << "Error reading document: " << e.what() << "\n";
    return 2;
  }

  Validator yavl(gr, doc);
  bool ok = yavl.validate();
  if (!ok) {
    cout << "ERRORS FOUND: " << endl << endl;
    cout << yavl.get_errors();
  }
  return !ok;
}


