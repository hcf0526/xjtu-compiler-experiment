//
// Created by WangLele on 25-5-21.
//

#ifndef CODE_HPP
#define CODE_HPP
#include "syntax.hpp"
#include "utils/json.hpp"


using json = nlohmann::json;
using TablePtr = Syntax::TablePtr;
using Pattern = std::regex;
using Replacement = std::string;

const std::string MIPS_REGEX_FILE = "input/code_regex/mips.json";

class Code {
public:
  Code(Syntax &syntax) : syntax_(syntax) {
    map_symbol_table_ = syntax_.symbol_table();
  }
  void to_three_addr_code(const std::string &filename);
  void to_mips(const std::string &filename);
  void parse_mips_regex(const std::string &filename);

private:
  Syntax &syntax_;
  std::unordered_map<std::string, TablePtr> map_symbol_table_;
  std::vector<std::pair<Pattern, Replacement>> mips_regex_rules_;

  static std::string table_to_three_addr_code(const TablePtr &table);
  std::string table_to_mips(const TablePtr &table);
  std::string three_addr_code_to_mips(const std::string &code_text);
  static std::string merge_code(const TablePtr &table);
};



#endif //CODE_HPP
