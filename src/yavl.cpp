#include <stdio.h>
#include <assert.h>
#include "yaml-cpp/yaml.h"
#include "yavl.h"

using namespace std;
using namespace YAVL;

namespace YAVL {
  template <>
  std::string ctype2str<unsigned long long>()
  {
    return "unsigned long long";
  }

  template <>
  std::string ctype2str<string>()
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

ostream& operator << (ostream& os, const Path& path)
{
  for (Path::const_iterator i = path.begin(); i != path.end(); ++i) {
    // no dot before list indexes and before first element
    if ((i != path.begin()) && ((*i)[0] != '[')) {
      os << '.';
    }
    os << *i;
  }
  return os;
}

ostream& operator << (ostream& os, const Exception& v)
{
  os << "REASON: " << v.why << endl;
  os << "  doc path: " << v.doc_path << endl;
  os << "  treespec path: " << v.gr_path << endl;
  os << endl;
  return os;
}

ostream& operator << (ostream& os, const Errors& v)
{
  for (Errors::const_iterator i = v.begin(); i != v.end(); ++i) {
    os << *i;
  }
  return os;
}

const string& Validator::type2str(YAML::NodeType::value t)
{
  static string undefinedstr = "undefined";
  static string nullstr = "null";
  static string scalarstr = "scalar";
  static string liststr = "list";
  static string mapstr = "map";

  assert( (t >= YAML::NodeType::Undefined) && (t <= YAML::NodeType::Map) );

  switch (t) {
    case YAML::NodeType::Undefined:
      return undefinedstr;
    case YAML::NodeType::Null:
      return nullstr;
    case YAML::NodeType::Scalar:
      return scalarstr;
    case YAML::NodeType::Sequence:
      return liststr;
    case YAML::NodeType::Map:
      return mapstr;
  }
  assert(0);
  return undefinedstr;
}

int Validator::num_keys(const YAML::Node& doc)
{
  if (doc.Type() != YAML::NodeType::Map) {
    return 0;
  }
  int num = 0;
  for (YAML::const_iterator i = doc.begin(); i != doc.end(); ++i) {
    num++;
  }
  return num;
}

bool Validator::validate_map(const YAML::Node &mapNode, const YAML::Node &doc)
{
  if (doc.Type() != YAML::NodeType::Map) {
    string reason = "expected map, but found " + type2str(doc.Type());
    gen_error(Exception(reason, gr_path, doc_path));
    return false;
  }

  bool ok = true;
  for (YAML::const_iterator i = mapNode.begin(); i != mapNode.end(); ++i) {
    string key = i->first.as<string>();
    const YAML::Node &valueNode = i->second;
    YAML::Node docMapNode;
    if (!(doc[key] && (docMapNode = doc[key]))) {
      string reason = "key: " + key + " not found.";
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

bool Validator::validate_leaf(const YAML::Node &gr, const YAML::Node &doc)
{
  assert( gr.Type() == YAML::NodeType::Sequence );

  const YAML::Node& typespec_map = gr[0];
  assert( num_keys(typespec_map) == 1);

  string type = typespec_map.begin()->first.as<string>();
  const YAML::Node& type_specifics = typespec_map.begin()->second;

  bool ok = true;
  if (type == "string") {
    attempt_to_convert<string>(doc, ok);
  } else if (type == "uint64") {
    attempt_to_convert<unsigned long long>(doc, ok);
  } else if (type == "int64") {
    attempt_to_convert<long long>(doc, ok);
  } else if (type == "int") {
    attempt_to_convert<int>(doc, ok);
  } else if (type == "uint") {
    attempt_to_convert<unsigned int>(doc, ok);
  } else if (type == "enum") {
    ok = false;
    string docValue = doc.as<std::string>();
    for (YAML::const_iterator i = type_specifics.begin(); i != type_specifics.end(); ++i) {
      if (i->as<std::string>() == docValue) {
        ok = true;
        break;
      }
    }
    if (!ok) {
      string reason = "enum string '" + docValue + "' is not allowed.";
      gen_error(Exception(reason, gr_path, doc_path));
    }
  }
  return ok;
}

bool Validator::validate_list(const YAML::Node &gr, const YAML::Node &doc)
{
  if (doc.Type() != YAML::NodeType::Sequence) {
    string reason = "expected list, but found " + type2str(doc.Type());
    gen_error(Exception(reason, gr_path, doc_path));
    return false;
  }

  bool ok = true;
  int n = 0;
  char buf[128];

  for (YAML::const_iterator i = doc.begin(); i != doc.end(); ++i, ++n) {
    snprintf(buf, sizeof(buf), "[%d]", n);
    doc_path.push_back(buf);
    ok = validate_doc(gr, *i) && ok;
    doc_path.pop_back();
  }
  return ok;
}

bool Validator::validate_doc(const YAML::Node &gr, const YAML::Node &doc)
{
  bool ok = true;
  YAML::Node mapNode;
  YAML::Node listNode;
  if (gr["map"] && (mapNode = gr["map"])) {
    gr_path.push_back("map");
    ok = validate_map(mapNode, doc) && ok;
    gr_path.pop_back();
  } else if (gr["list"] && (listNode = gr["list"])) {
    gr_path.push_back("list");
    ok = validate_list(listNode, doc) && ok;
    gr_path.pop_back();
  } else {
    ok = validate_leaf(gr, doc) && ok;
  }
  return ok;
}
