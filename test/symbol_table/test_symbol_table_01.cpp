//
// Created by WangLele on 25-5-1.
//
#include "syntax.hpp"
#include "basic/item.hpp"
#include "basic/grammar.hpp"
#include "basic/slr_table.hpp"
#include "basic/code.hpp"

int main() {
  std::string program_file = index_format("input/program/program", 3, ".txt");

  // 词法分析
  Lexical lexical(LEXICAL_EXTEND);
  auto tokens = lexical.analyze(program_file);
  lexical.to_txt("output/lexical/lexical.txt");

  // 文法集
  GrammarSet grammar_set(GRAMMAR_EXTEND, "P");
  ItemCluster item_cluster(grammar_set);
  item_cluster.build();

  // SLR分析表
  SLRTable slr_table(item_cluster);
  slr_table.read_csv(SLR_TABLE_EXTEND);

  // 符号表分析
  SyntaxZyl syntax(slr_table);
  syntax.parse(tokens);
  syntax.symbol_table_to_txt("output/symbol_table/symbol_table.txt");

  // 代码输出
  Code code(syntax);
  code.to_three_addr_code("output/code/three_addr_code.txt");
  return 0;
}

/*
// 获取文法编号(暂时用)
std::string result = "/grammar.txt";
auto grammar_to_id = slr_table.grammar_to_id();
std::vector<std::pair<int, Grammar>> grammar_list;
auto grammars = gs.grammars();
for (const auto& [name, grammar_vector] : grammars) {
  for (const auto& grammar : grammar_vector) {
    int id = grammar_to_id.at(grammar);
    grammar_list.emplace_back(id, grammar);
  }
}
std::sort(grammar_list.begin(), grammar_list.end(),
        [](const auto& a, const auto& b) {
          return a.first < b.first;
        });
for (const auto & [id, grammar] : grammar_list) {
  std::cout << id << ": " << grammar << std::endl;
}
*/