//
// Created by WangLele on 25-4-26.
//
#include "basic/item.hpp"
#include "basic/grammar.hpp"
#include "basic/slr_table.hpp"

int main() {
  constexpr int index = 1;

  std::string gm_file = index_format("input/grammar/grammar", index, ".txt");
  std::string ic_file = index_format("output/item_cluster/item_cluster", index, ".json");
  std::string ic_file_ = index_format("output/item_cluster/item_cluster", index, ".txt");
  std::string slr_file = index_format("output/slr_table/slr_table", index, ".csv");

  GrammarSet gs(gm_file, "P");

  ItemCluster ic(gs);
  ic.build();
  ic.to_dot(index_format("output/item_cluster/item_cluster", index, ".dot"));
  // ic.to_json(ic_file);
  // ic.to_txt(ic_file_);

  // SLRTable slr_table(ic);
  // slr_table.build();
  // // 输出冲突状态
  // auto id_to_state = slr_table.id_to_state();
  // auto conflict_states = slr_table.conflicts();
  // for (const auto& conflict : conflict_states) {
  //   const auto& state_name = id_to_state.at(conflict.state);
  //   std::cout << state_name << ": " << std::endl;
  //   std::cout << "  " << conflict.symbol << ": ";
  //   for (const auto& action : conflict.actions) {
  //     std::cout << action << " ";
  //   }
  //   std::cout << std::endl;
  //   const auto& item_cluster_state = ic[state_name];
  //   auto item_cluster_state_closure = item_cluster_state.closure;
  //   std::cout << item_cluster_state_closure << std::endl;
  // }
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