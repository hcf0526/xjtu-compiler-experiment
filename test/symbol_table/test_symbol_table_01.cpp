//
// Created by WangLele on 25-5-1.
//
#include "syntax.hpp"
#include "basic/item.hpp"
#include "basic/grammar.hpp"
#include "basic/slr_table.hpp"

int main() {
  constexpr int index = 1;

  std::string regex_file = index_format("input/lexical/lexical", 1, ".json");
  std::string gm_file = index_format("input/grammar/grammar", index, ".txt");
  std::string slr_file = index_format("input/slr_table/slr_table", index, ".csv");
  std::string tokens_file = index_format("output/lexical/lexical", 2, ".txt");
  std::string program_file = index_format("input/program/program", 2, ".txt");
  std::string processes_file = index_format("output/symbol_table/processes", index, ".txt");
  std::string st_file = index_format("output/symbol_table/symbol_table", index, ".txt");

  // 词法分析
  Lexical lexical(regex_file);
  auto tokens = lexical.analyze(program_file);
  // lexical.to_txt(tokens_file);

  // 文法集
  GrammarSet gs(gm_file, "P");
  ItemCluster ic(gs);
  ic.build();

  // SLR分析表
  SLRTable slr_table(ic);
  slr_table.read_csv(slr_file);

  // 符号表分析
  SyntaxZyl sta1(slr_table);
  sta1.parse(tokens);
  sta1.processes_to_txt(processes_file);
  sta1.symbol_table_to_txt(st_file);

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