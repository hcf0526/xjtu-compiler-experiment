//
// Created by WangLele on 25-5-1.
//
#include "basic/lexical.hpp"
#include "utils/format.hpp"
#include <iostream>

int main() {
  // constexpr int index = 1;
  std::string regex_file = index_format("input/lexical/lexical", 1, ".json");
  std::string program_file = index_format("input/program/program", 2, ".txt");
  std::string lexical_file = index_format("output/lexical/lexical", 2, ".txt");
  Lexical lexical(regex_file);
  lexical.analyze(program_file);
  lexical.write_tokens(lexical_file);
}