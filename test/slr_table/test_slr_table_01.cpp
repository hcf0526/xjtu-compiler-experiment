//
// Created by WangLele on 25-4-26.
//
#include "basic/item.hpp"
#include "basic/grammar.hpp"
#include "basic/slr_table.hpp"

int main() {
  GrammarSet grammar_set(GRAMMAR_EXTEND, "P");

  ItemCluster item_cluster(grammar_set);
  item_cluster.build();
  item_cluster.to_txt("output/item_cluster/item_cluster.txt");

  SLRTable slr_table(item_cluster);
  slr_table.build();
  slr_table.to_csv("output/slr_table/slr_table_extend_before.csv");

  // 输出冲突状态
  // slr_table.conflict_to_csv("output/slr_table/conflict_before.csv");

  // 获取文法编号
  // slr_table.id_to_grammar_to_txt("output/slr_table/id_to_grammar.txt");

  // 消除冲突状态
  slr_table.eliminate_conflict("output/slr_table/conflict_after.csv");
  slr_table.to_csv("output/slr_table/slr_table_extend_after.csv");
  return 0;
}

/*
// 输出冲突状态
auto id_to_state = slr_table.id_to_state();
auto conflict_states = slr_table.conflicts();
for (const auto& conflict : conflict_states) {
  const auto& state_name = id_to_state.at(conflict.state);
  std::cout << state_name << ": " << std::endl;
  std::cout << "  " << conflict.symbol << ": ";
  for (const auto& action : conflict.actions) {
    std::cout << action << " ";
  }
  std::cout << std::endl;
  const auto& item_cluster_state = ic[state_name];
  auto item_cluster_state_closure = item_cluster_state.closure;
  std::cout << item_cluster_state_closure << std::endl;
}
*/

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