//
// Created by WangLele on 25-5-6.
//

#include "compiler_signal.hpp"
#include "basic/lexical.hpp"
#include "basic/grammar.hpp"
#include "basic/item.hpp"
#include "basic/slr_table.hpp"
#include "basic/syntax.hpp"
#include <QDir>



SyntaxZyl analyse_syntax(Text program, Text lexical, Text grammar, Text slr_table) {
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

SLRTable analyse_slr_table(Text grammar) {
  GrammarSet grammar_set(grammar);
  ItemCluster item_cluster(grammar_set);
  item_cluster.build();

  SLRTable slr_table(item_cluster);
  slr_table.build();
  return slr_table;
}

bool parse_syntax(Text file, Text result) {
  int success = 0;
  QString lexical;
  QString grammar;
  QString slr_table;

  if (file_default_lexical.open(QIODevice::ReadOnly | QIODevice::Text)) {
    lexical = file_default_lexical.readAll();
    success++;
  }
  if (file_default_grammar.open(QIODevice::ReadOnly | QIODevice::Text)) {
    grammar = file_default_lexical.readAll();
    success++;
  }
  if (file_default_slr_table.open(QIODevice::ReadOnly | QIODevice::Text)) {
    slr_table = file_default_lexical.readAll();
    success++;
  }
  if (success != 3) return false;
  std::ifstream if_program(file);
  if (!if_program) return false;
  std::stringstream str_program;
  str_program << if_program.rdbuf();
  std::string program = str_program.str();

  SyntaxZyl syntax = analyse_syntax(program, lexical.toStdString(), grammar.toStdString(), slr_table.toStdString());
  syntax.processes_to_txt(result + "/processes.txt");
  syntax.symbol_table_to_txt(result + "/symbol_table.txt");
  return true;
}

bool parse_slr_table(QText filename_grammar, QText result) {
  int success = 0;
  QString grammar;

  QFile file_grammar(filename_grammar.isEmpty() ? filename_default_grammar : filename_grammar);

  if (file_grammar.open(QIODevice::ReadOnly | QIODevice::Text)) {
    grammar = file_grammar.readAll();
    success++;
  }

  if (success != 1) return false;

  SLRTable slr_table = analyse_slr_table(grammar.toStdString());

  QString full_path = QDir(result).filePath("slr_table.csv");
  slr_table.to_csv(full_path.toStdString());
  return true;
}