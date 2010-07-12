#ifndef YAVL_H_
#define YAVL_H_

#include "yaml.h"
#include <vector>
#include <string>
#include <ostream>

typedef std::vector<std::string> Path;

class YAVL_Exception {
public:
  std::string why;
  Path gr_path;
  Path doc_path;
  YAVL_Exception(const std::string _why,
    const Path& _gr_path,
    const Path& _doc_path) :
      why(_why), gr_path(_gr_path), doc_path(_doc_path) {};
};

typedef std::vector<YAVL_Exception> Errors;

class YAVL {
  const YAML::Node& gr;
  const YAML::Node& doc;
  Path gr_path;
  Path doc_path;
  Errors errors;

  int num_keys(const YAML::Node& doc);
  const std::string& type2str(YAML::CONTENT_TYPE t);
  bool validate_map(const YAML::Node &mapNode, const YAML::Node &doc);
  bool validate_leaf(const YAML::Node &gr, const YAML::Node &doc);
  bool validate_list(const YAML::Node &gr, const YAML::Node &doc);
  bool validate_doc(const YAML::Node &gr, const YAML::Node &doc);

  void gen_error(const YAVL_Exception& err) {
    errors.push_back(err);
  }

public:
  YAVL(const YAML::Node& _gr, const YAML::Node& _doc) :
    gr(_gr), doc(_doc) {};
  bool validate() {
    return validate_doc(gr, doc);
  }
  const Errors& get_errors() {
    return errors;
  }
};

std::ostream& operator << (std::ostream& os, const Path& path);
std::ostream& operator << (std::ostream& os, const YAVL_Exception& v);
std::ostream& operator << (std::ostream& os, const Errors& v);

#endif

