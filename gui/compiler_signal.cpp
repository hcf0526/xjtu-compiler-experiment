//
// Created by WangLele on 25-5-6.
//

#include "compiler_signal.hpp"
#include "basic/lexical.hpp"
#include "basic/grammar.hpp"
#include "basic/item.hpp"
#include "basic/slr_table.hpp"
#include "basic/syntax.hpp"

SyntaxZyl analyse(Text program, Text lexical, Text grammar, Text slr_table) {
  // 词法分析
  Lexical lex(lexical);
  auto tokens = lex.analyze(program);

  // 文法集
  GrammarSet gms(grammar);

  // 项目簇
  ItemCluster ic(gms);
  ic.build();

  // SLR 分析表
  SLRTable slr(ic);
  slr.read_csv(slr_table);

  // 符号表分析
  SyntaxZyl syntax(slr);
  syntax.parse(tokens);
  return syntax;
}

bool parse(Text file, Text result) {
  int success = 0;
  QString lexical;
  QString grammar;
  QString slr_table;

  if (default_lexical.open(QIODevice::ReadOnly | QIODevice::Text)) {
    lexical = default_lexical.readAll();
    success++;
  }
  if (default_grammar.open(QIODevice::ReadOnly | QIODevice::Text)) {
    grammar = default_lexical.readAll();
    success++;
  }
  if (default_slr_table.open(QIODevice::ReadOnly | QIODevice::Text)) {
    slr_table = default_lexical.readAll();
    success++;
  }
  if (success != 3) return false;
  std::ifstream if_program(file);
  if (!if_program) return false;
  std::stringstream str_program;
  str_program << if_program.rdbuf();
  std::string program = str_program.str();

  SyntaxZyl syntax = analyse(program, lexical.toStdString(), grammar.toStdString(), slr_table.toStdString());
  syntax.processes_to_txt(result + "processes.txt");
  syntax.symbol_table_to_txt(result + "symbol_table.txt");
  return true;
}