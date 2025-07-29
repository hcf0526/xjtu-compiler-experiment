//
// Created by WangLele on 25-5-21.
//

#include "code.hpp"

// 三地址代码

void Code::to_three_addr_code(const std::string &filename) {
  std::ofstream out(filename);
  if (!out.is_open()) {
    std::cerr << "[Code] 无法打开文件" << filename << std::endl;
    return;
  }
  // 先加 system_table.code
  TablePtr system_table = map_symbol_table_["system_table"];
  out << table_to_three_addr_code(system_table);
  for (const auto& [name, table] : map_symbol_table_) {
    if (name != "system_table") out << table_to_three_addr_code(table);
  }
}

std::string Code::table_to_three_addr_code(const TablePtr &table) {
  std::ostringstream out;
  std::string line;
  std::istringstream iss(merge_code(table));

  if (table->name != "system_table") {
    out << "LABEL " << table->name << ";\n";
  }

  while (std::getline(iss, line)) {
    if (line.find("LABEL") == std::string::npos) {
      out << "  " << line << '\n';
    } else {
      out << line << '\n';
    }
  }
  return out.str();
}

// MIPS

void Code::parse_mips_regex(const std::string &filename) {
  std::ifstream in(filename);
  if (!in.is_open()) {
    throw std::runtime_error("[Code] 无法打开 json 文件: " + filename);
  }

  json j;
  in >> j;

  mips_regex_rules_.clear();

  std::vector<std::pair<int, std::pair<Pattern, Replacement>>> rules_with_priority;

  for (const auto& [priority_str, rule_group] : j.items()) {
    int priority = std::stoi(priority_str);
    for (const auto& [rule_name, rule_obj] : rule_group.items()) {
      std::string pattern_str = rule_obj["pattern"];
      std::string replacement = rule_obj["replacement"];

      Pattern compiled("^" + pattern_str);
      rules_with_priority.emplace_back(priority, std::make_pair(compiled, replacement));
    }
  }

  std::sort(rules_with_priority.begin(), rules_with_priority.end(),
  [](const auto& a, const auto& b) {
    return a.first < b.first;
  });

  for (const auto& [priority, pair] : rules_with_priority) {
    mips_regex_rules_.push_back(pair);
  }
}

void Code::to_mips(const std::string &filename) {
  std::ofstream out(filename);
  if (!out.is_open()) {
    std::cerr << "[Code] 无法打开文件" << filename << std::endl;
    return;
  }
  // 先加 system_table.code
  TablePtr system_table = map_symbol_table_["system_table"];
  out << table_to_mips(system_table);
  for (const auto& [name, table] : map_symbol_table_) {
    if (name != "system_table") out << table_to_mips(table);
  }
}

std::string Code::table_to_mips(const TablePtr &table) {
  std::ostringstream out;
  std::string line;
  std::istringstream iss(merge_code(table));

  if (table->name != "system_table") {
    out << table->name << ":\n";
  }

  // 参数出栈
  std::vector<std::string> arglist = table->arglist;
  for (const auto& name : arglist) {
    out << "  lw $" << name << ", 0($sp)\n";
    out << "  addi $sp, $sp, 4\n";
  }

  out << three_addr_code_to_mips(iss.str());
  return out.str();
}

std::string Code::three_addr_code_to_mips(const std::string &code_text) {
  std::istringstream in(code_text);
  std::ostringstream out;

  std::string line;
  int line_number = 1;

  while (std::getline(in, line)) {
    line.erase(line.begin(), std::find_if_not(line.begin(), line.end(), ::isspace));

    bool matched = false;

    for (const auto& [pattern, replacement] : mips_regex_rules_) {
      std::smatch match;
      if (std::regex_match(line, match, pattern)) {
        std::string result = replacement;

        // 替换 \$1, \$2,... 占位符（正则组）
        for (size_t i = 1; i < match.size(); ++i) {
          // 替换 $1
          std::string placeholder_plain = "\\$" + std::to_string(i);
          result = std::regex_replace(result, std::regex(placeholder_plain), match[i].str());

          // 替换 $t$1 形式（寄存器拼接）
          std::string placeholder_nested = "\\$([a-zA-Z]*)\\$" + std::to_string(i);
          result = std::regex_replace(result, std::regex(placeholder_nested),
                                      match[i].str().empty() ? "" : "$$1" + match[i].str());
        }

        // 把\$替换为$
        result = std::regex_replace(result, std::regex(R"(\\\$)"), "$");
        // 最后把 \n 字符串替换为换行符
        result = std::regex_replace(result, std::regex(R"(\\n)"), "\n");

        std::istringstream result_stream(result);
        std::string mips_line;
        while (std::getline(result_stream, mips_line)) {
          if (mips_line.find(':') != std::string::npos) {
            out << mips_line << '\n';
          } else {
            out << "  " << mips_line << '\n';
          }
        }

        matched = true;
        break;
      }
    }

    if (!matched && !line.empty()) {
      std::cerr << "[Code] " << line_number << ": 未识别的语句 " << line << '\n';
    }

    ++line_number;
  }

  return out.str();
}

std::string Code::merge_code(const TablePtr &table) {
  std::ostringstream result;
  for (const auto& code : table->code) {
    result << code;
  }
  return result.str();
}
