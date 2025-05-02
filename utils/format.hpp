#ifndef FORMAT_HPP
#define FORMAT_HPP
#include <string>
#include <ostream>
#include "utils/json.hpp"

using json = nlohmann::json;

inline std::string index_format(const std::string &filename, const int index, const std::string &suffix) {
  return std::string(filename)
        + std::string("_")
        + (index < 10 ? "0" : "")
        + std::to_string(index)
        + std::string(suffix);
}

// 自定义Serializer，控制换行
struct compact_serializer {
  template<typename BasicJsonType>
  static void dump(const BasicJsonType& j, std::ostream& o, bool pretty_print,
                   unsigned int indent_step, unsigned int current_indent = 0) {
    if (j.is_object()) {
      o << "{";
      bool first = true;
      for (auto it = j.cbegin(); it != j.cend(); ++it) {
        if (!first) {
          o << ",";
        }
        o << "\n" << std::string(current_indent + indent_step, ' ');
        o << "\"" << it.key() << "\": ";
        dump(it.value(), o, pretty_print, indent_step, current_indent + indent_step);
        first = false;
      }
      if (!j.empty()) {
        o << "\n" << std::string(current_indent, ' ');
      }
      o << "}";
    }
    else if (j.is_array()) {
      o << "[";
      bool first = true;
      for (auto it = j.begin(); it != j.end(); ++it) {
        if (!first) {
          o << ", ";
        }
        dump(*it, o, false, indent_step, current_indent);
        first = false;
      }
      o << "]";
    }
    else {
      // 基本类型：数字、字符串、布尔
      o << j;
    }
  }
};
#endif // FORMAT_HPP