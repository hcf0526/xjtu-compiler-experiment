//
// Created by WangLele on 25-5-1.
//
#include "basic/lexical.hpp"
#include "utils/format.hpp"

int main() {
  std::string program_file = index_format("input/program/program", 3, ".txt");

  Lexical lexical(LEXICAL_EXTEND);
  lexical.analyze(program_file);
  lexical.to_txt("output/lexical/lexical.txt");
}