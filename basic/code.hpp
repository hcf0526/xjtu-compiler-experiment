//
// Created by WangLele on 25-5-21.
//

#ifndef CODE_HPP
#define CODE_HPP
#include "syntax.hpp"

using TablePtr = Syntax::TablePtr;

class Code {
public:
  Code(Syntax &syntax) : syntax_(syntax) {
    map_symbol_table_ = syntax_.symbol_table();
  }
  void to_three_addr_code(const std::string &filename);
private:
  Syntax &syntax_;
  std::unordered_map<std::string, TablePtr> map_symbol_table_;

  static void add_three_addr_code(std::ofstream &out, const TablePtr &table);
  static std::string merge_code(const TablePtr &table);
};



#endif //CODE_HPP
