// FIXME we need license/copyright here

#include <stdio.h>
#include <assert.h>

#include "yaml-cpp/yaml.h"
#include "yavl.h"

namespace YAVL {
  template <>
  std::string ctype2str<unsigned long long>()
  {
    return "unsigned long long";
  }

  template <>
  std::string ctype2str<std::string>()
  {
    return "string";
  }

  template <>
  std::string ctype2str<long long>()
  {
    return "long long";
  }

  template <>
  std::string ctype2str<unsigned int>()
  {
    return "unsigned int";
  }

  template <>
  std::string ctype2str<int>()
  {
    return "int";
  }

}

std::ostream& operator << (std::ostream& os, const YAVL::Path& path)
{
  for (YAVL::Path::const_iterator i = path.begin(); i != path.end(); ++i) {
    // no dot before list indexes and before first element
    if ((i != path.begin()) && ((*i)[0] != '[')) {
      os << '.';
    }
    os << *i;
  }
  return os;
}

std::ostream& operator << (std::ostream& os, const YAVL::Exception& v)
{
  os << "REASON: " << v.why << std::endl;
  os << "  doc path: " << v.doc_path << std::endl;
  os << "  treespec path: " << v.gr_path << std::endl;
  os << std::endl;
  return os;
}

std::ostream& operator << (std::ostream& os, const YAVL::Errors& v)
{
  for (YAVL::Errors::const_iterator i = v.begin(); i != v.end(); ++i) {
    os << *i;
  }
  return os;
}

namespace YAVL {

const std::string &Validator::type2str(YAML::NodeType::value t) {
  static std::string nonestr = "none";
  static std::string scalarstr = "scalar";
  static std::string liststr = "list";
  static std::string mapstr = "map";

  assert((t >= YAML::NodeType::Null) && (t <= YAML::NodeType::Null));

  switch (t) {
  default:
    assert(0); // FIXME
  case YAML::NodeType::Null:
    return nonestr;
  case YAML::NodeType::Scalar:
    return scalarstr;
  case YAML::NodeType::Sequence:
    return liststr;
  case YAML::NodeType::Map:
    return mapstr;
  }
  assert(0);
  return nonestr;
}

int Validator::num_keys(const YAML::Node &document) {
  if (document.Type() != YAML::NodeType::Map) {
    return 0;
  }
  int num = 0;
  for (YAML::const_iterator i = document.begin(); i != document.end(); ++i) {
    num++;
  }
  return num;
}

bool Validator::validate_map(const YAML::Node &mapNode,
                             const YAML::Node &document) {
  if (document.Type() != YAML::NodeType::Map) {
    std::string reason = "expected map, but found " + type2str(document.Type());
    gen_error(Exception(reason, gr_path, doc_path));
    return false;
  }

  bool ok = true;
  for (YAML::const_iterator i = mapNode.begin(); i != mapNode.end(); ++i) {
    std::string key = i->first.as<std::string>();
    const YAML::Node &valueNode = i->second;
    const YAML::Node &docMapNode = document[key];
    if (!docMapNode) {
      std::string reason = "key: " + key + " not found.";
      gen_error(Exception(reason, gr_path, doc_path));
      ok = false;
    } else {
      doc_path.push_back(key);
      gr_path.push_back(key);

      ok = validate_doc(valueNode, docMapNode) && ok;

      gr_path.pop_back();
      doc_path.pop_back();
    }
  }
  return ok;
}

bool Validator::validate_leaf(const YAML::Node &gr,
                              const YAML::Node &document) {
  assert(gr.Type() == YAML::NodeType::Sequence);

  const YAML::Node &typespec_map = gr[0];
  assert(num_keys(typespec_map) == 1);

  std::string type = typespec_map.begin()->first.as<std::string>();
  const YAML::Node &type_specifics = typespec_map.begin()->second;

  bool ok = true;
  if (type == "string") {
    attempt_to_convert<std::string>(document, ok);
  } else if (type == "uint64") {
    attempt_to_convert<unsigned long long>(document, ok);
  } else if (type == "int64") {
    attempt_to_convert<long long>(document, ok);
  } else if (type == "int") {
    attempt_to_convert<int>(document, ok);
  } else if (type == "uint") {
    attempt_to_convert<unsigned int>(document, ok);
  } else if (type == "enum") {
    ok = false;
    std::string docValue = document.as<std::string>();
    for (YAML::const_iterator i = type_specifics.begin();
         i != type_specifics.end(); ++i) {
      if (i->as<std::string>() == docValue) {
        ok = true;
        break;
      }
    }
    if (!ok) {
      std::string reason =
          "enum std::string '" + docValue + "' is not allowed.";
      gen_error(Exception(reason, gr_path, doc_path));
    }
  }
  return ok;
}

bool Validator::validate_list(const YAML::Node &gr, const YAML::Node &document) {
  if (document.Type() != YAML::NodeType::Sequence) {
    std::string reason = "expected list, but found " + type2str(document.Type());
    gen_error(Exception(reason, gr_path, doc_path));
    return false;
  }

  bool ok = true;
  int n = 0;
  char buf[128];

  for (YAML::const_iterator i = document.begin(); i != document.end(); ++i, ++n) {
    snprintf(buf, sizeof(buf), "[%d]", n);
    doc_path.push_back(buf);
    ok = validate_doc(gr, *i) && ok;
    doc_path.pop_back();
  }
  return ok;
}

bool Validator::validate_doc(const YAML::Node &gr, const YAML::Node &document) {
  bool ok = true;
  const YAML::Node &mapNode = gr["map"];
  const YAML::Node &listNode = gr["list"];
  if (mapNode) {
    gr_path.push_back("map");
    ok = validate_map(mapNode, document) && ok;
    gr_path.pop_back();
  } else if (listNode) {
    gr_path.push_back("list");
    ok = validate_list(listNode, document) && ok;
    gr_path.pop_back();
  } else {
    ok = validate_leaf(gr, document) && ok;
  }
  return ok;
}

} // namespace YAVL