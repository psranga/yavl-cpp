#include <stdio.h>
#include <assert.h>
#include "yaml.h"
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

const string& Validator::type2str(YAML::CONTENT_TYPE t)
{
  static string nonestr = "none";
  static string scalarstr = "scalar";
  static string liststr = "list";
  static string mapstr = "map";

  assert( (t >= YAML::CT_NONE) && (t <= YAML::CT_MAP) );

  switch (t) {
    case YAML::CT_NONE:
      return nonestr;
    case YAML::CT_SCALAR:
      return scalarstr;
    case YAML::CT_SEQUENCE:
      return liststr;
    case YAML::CT_MAP:
      return mapstr;
  }
  assert(0);
  return nonestr;
}

int Validator::num_keys(const YAML::Node& doc)
{
  if (doc.GetType() != YAML::CT_MAP) {
    return 0;
  }
  int num = 0;
  for (YAML::Iterator i = doc.begin(); i != doc.end(); ++i) {
    num++;
  }
  return num;
}

bool Validator::validate_map(const YAML::Node &mapNode, const YAML::Node &doc)
{
  if (doc.GetType() != YAML::CT_MAP) {
    string reason = "expected map, but found " + type2str(doc.GetType());
    gen_error(Exception(reason, gr_path, doc_path));
    return false;
  }

  bool ok = true;
  for (YAML::Iterator i = mapNode.begin(); i != mapNode.end(); ++i) {
    string key = i.first();
    const YAML::Node &valueNode = i.second();
    const YAML::Node *docMapNode = 0;
    if (!(docMapNode = doc.FindValue(key))) {
      string reason = "key: " + key + " not found.";
      gen_error(Exception(reason, gr_path, doc_path));
      ok = false;
    } else {
      doc_path.push_back(key);
      gr_path.push_back(key);

      ok = validate_doc(valueNode, *docMapNode) && ok;

      gr_path.pop_back();
      doc_path.pop_back();
    }
  }
  return ok;
}

bool Validator::validate_leaf(const YAML::Node &gr, const YAML::Node &doc)
{
  assert( gr.GetType() == YAML::CT_SEQUENCE );

  const YAML::Node& typespec_map = gr[0];
  assert( num_keys(typespec_map) == 1);

  string type = typespec_map.begin().first();
  const YAML::Node& type_specifics = typespec_map.begin().second();

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
    string docValue = doc;
    for (YAML::Iterator i = type_specifics.begin(); i != type_specifics.end(); ++i) {
      if (*i == docValue) {
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
  if (doc.GetType() != YAML::CT_SEQUENCE) {
    string reason = "expected list, but found " + type2str(doc.GetType());
    gen_error(Exception(reason, gr_path, doc_path));
    return false;
  }

  bool ok = true;
  int n = 0;
  char buf[128];

  for (YAML::Iterator i = doc.begin(); i != doc.end(); ++i, ++n) {
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
  const YAML::Node *mapNode = 0;
  const YAML::Node *listNode = 0;
  if ((mapNode = gr.FindValue("map"))) {
    gr_path.push_back("map");
    ok = validate_map(*mapNode, doc) && ok;
    gr_path.pop_back();
  } else if ((listNode = gr.FindValue("list"))) {
    gr_path.push_back("list");
    ok = validate_list(*listNode, doc) && ok;
    gr_path.pop_back();
  } else {
    ok = validate_leaf(gr, doc) && ok;
  }
  return ok;
}
