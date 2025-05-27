//
// Created by WangLele on 25-5-21.
//

#include "code.hpp"

void Code::to_three_addr_code(const std::string &filename) {
  std::ofstream out(filename);
  if (!out.is_open()) {
    std::cerr << "[Code] 无法打开文件" << filename << std::endl;
    return;
  }
  // 先加 system_table.code
  TablePtr system_table = map_symbol_table_["system_table"];
  add_three_addr_code(out, system_table);
  for (const auto& [name, table] : map_symbol_table_) {
    if (name != "system_table") add_three_addr_code(out, table);
  }
}

void Code::add_three_addr_code(std::ofstream &out, const TablePtr &table) {
  std::string line;
  std::istringstream iss(merge_code(table));
  if (table->name != "system_table") {
    out << std::string("LABEL ") << table->name << std::endl;
  }
  while (std::getline(iss, line)) {
    if (line.find("LABEL") == std::string::npos) {
      out << "  " << line << std::endl;
    } else {
      out<< line << std::endl;
    }
  }
}

std::string Code::merge_code(const TablePtr &table) {
  std::ostringstream result;
  for (const auto& code : table->code) {
    result << code;
  }
  return result.str();
}
