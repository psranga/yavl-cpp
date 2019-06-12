// FIXME we need license/copyright here

#ifndef _YAVL_H_
#define _YAVL_H_

#include <vector>
#include <string>
#include <ostream>

#include "yaml-cpp/yaml.h"
#include "yaml-cpp/node/node.h"

namespace YAVL
{

  typedef std::vector<std::string> Path;

  // really sucks that I have to do this sort of crap since I can't
  // pass a type as an argument to a function.
  template <typename T>
  std::string ctype2str()
  {
    return "FAIL";
  }

  class Exception {
  public:
    std::string why;
    Path gr_path;
    Path doc_path;
    Exception(const std::string _why,
      const Path& _gr_path,
      const Path& _doc_path) :
        why(_why), gr_path(_gr_path), doc_path(_doc_path) {};
  };

  typedef std::vector<Exception> Errors;

  class Validator {
    const YAML::Node& _gr;
    const YAML::Node& doc;
    Path gr_path;
    Path doc_path;
    Errors errors;

    int num_keys(const YAML::Node& document);
    const std::string& type2str(YAML::NodeType::value t);
    bool validate_map(const YAML::Node &mapNode, const YAML::Node &document);
    bool validate_leaf(const YAML::Node &gr, const YAML::Node &document);
    bool validate_list(const YAML::Node &gr, const YAML::Node &document);
    bool validate_doc(const YAML::Node &gr, const YAML::Node &document);

    void gen_error(const Exception& err) {
      errors.push_back(err);
    }

    template<typename T>
    void attempt_to_convert(const YAML::Node& scalar_node, bool& ok) {
      try {
        (void) scalar_node.as<T>();
        ok = true;
      } catch (const YAML::InvalidScalar& e) {
        std::string s = scalar_node.as<std::string>();
        std::string reason = "unable to convert '" + s + "' to '" + YAVL::ctype2str<T>() + "'.";
        gen_error(Exception(reason, gr_path, doc_path));
        ok = false;
      }
    }

  public:
    Validator(const YAML::Node& gr, const YAML::Node& _doc) :
      _gr(gr), doc(_doc) {};
    bool validate() {
      return validate_doc(_gr, doc);
    }
    const Errors& get_errors() {
      return errors;
    }
  };
}

std::ostream& operator << (std::ostream& os, const YAVL::Path& path);
std::ostream& operator << (std::ostream& os, const YAVL::Exception& v);
std::ostream& operator << (std::ostream& os, const YAVL::Errors& v);

#endif

