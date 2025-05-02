//
// Created by WangLele on 25-4-26.
//
#include "symbol_table_analyzer.hpp"
#include "basic/item.hpp"
#include "basic/grammar.hpp"
#include "basic/slr_table.hpp"

int main() {
  constexpr int index = 1;

  std::string gm_file = index_format("input/grammar/grammar", index, ".txt");
  std::string slr_file = index_format("input/slr_table/slr_table", index, ".csv");
  std::string regex_file = index_format("input/lexical/lexical", 1, ".json");
  std::string program_file = index_format("input/program/program", 2, ".txt");
  std::string st_file = index_format("output/symbol_table/symbol_table", index, ".txt");

  // 词法分析
  Lexical lexical(regex_file);
  auto tokens = lexical.analyze(program_file);

  // 文法集
  GrammarSet gs(gm_file, "P");
  ItemCluster ic(gs);
  ic.build();

  // SLR分析表
  SLRTable slr_table(ic);
  slr_table.read_csv(slr_file);

  // 符号表分析
  SymbolTableAnalyzer1 sta1(slr_table);
  sta1.parse(tokens);
  auto symbol_table = sta1.symbol_table();
  for (const auto& [name, table] : symbol_table) {
    std::cout << *table << std::endl;
  }
  std::cout << std::endl;
  // sta1.to_txt(st_file);

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