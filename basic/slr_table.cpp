#include "slr_table.hpp"
#include "utils/csv.hpp"
#include <set>

SLRTable::SLRTable(const ItemCluster& cluster): item_cluster_(cluster) {
  assign_ids(cluster);
  compute_states(cluster);
}

const SLRTable::ActionSet* SLRTable::get_action(int state, const std::string& symbol) const {
  auto it1 = action_table_.find(state);
  if (it1 == action_table_.end()) return nullptr;
  auto it2 = it1->second.find(symbol);
  if (it2 == it1->second.end()) return nullptr;
  return &(it2->second);
}

SLRTable::ActionRow SLRTable::get_action(int state) const {
  auto it = action_table_.find(state);
  if (it != action_table_.end()) {
    return it->second;
  }
  return {};
}

std::unordered_map<int, SLRTable::ActionRow> SLRTable::get_action() const {
  return action_table_;
}

const SLRTable::GotoSet* SLRTable::get_goto(int state, const std::string& non_terminal) const {
  auto it1 = goto_table_.find(state);
  if (it1 == goto_table_.end()) return nullptr;
  auto it2 = it1->second.find(non_terminal);
  if (it2 == it1->second.end()) return nullptr;
  return &(it2->second);
}

SLRTable::GotoRow SLRTable::get_goto(int state) const {
  auto it = goto_table_.find(state);
  if (it != goto_table_.end()) {
    return it->second;
  }
  return {};
}

std::unordered_map<int, SLRTable::GotoRow> SLRTable::get_goto() const {
  return goto_table_;
}

void SLRTable::add_action(int state, const std::string& symbol, ActionType type, int target) {
  Action new_action{type, target};
  action_table_[state][symbol].insert(new_action);
}

void SLRTable::add_goto(int state, const std::string& non_terminal, int next_state) {
  goto_table_[state][non_terminal].insert(next_state);
}

int SLRTable::start_state() const {
  return start_state_;
}

const std::unordered_set<int>& SLRTable::accept_states() const {
  return accept_states_;
}

int SLRTable::final_accept_state() const {
  return final_accept_state_;
}

const SLRTable::ConflictSet& SLRTable::conflicts() const {
  return conflicts_;
}

const std::unordered_map<std::string, int>& SLRTable::state_to_id() const {
  return state_to_id_;
}

const std::unordered_map<Grammar, int, Grammar::Hash>& SLRTable::grammar_to_id() const {
  return grammar_to_id_;
}

const std::unordered_map<int, std::string> SLRTable::id_to_state() const {
  return id_to_state_;
}

const std::unordered_map<int, Grammar> SLRTable::id_to_grammar() const {
  return id_to_grammar_;
}

std::string SLRTable::find_state(const int id) const {
  auto it = id_to_state_.find(id);
  if (it != id_to_state_.end()) return it->second;
  throw std::out_of_range("State ID not found.");
}

int SLRTable::find_state(const std::string& name) const {
  auto it = state_to_id_.find(name);
  if (it != state_to_id_.end()) return it->second;
  throw std::out_of_range("State name not found.");
}

ItemCluster::State SLRTable::find_item_set(const int id) const {
  const std::string name = find_state(id);
  return item_cluster_[name];
}

Grammar SLRTable::find_grammar(const int id) const {
  return id_to_grammar_.at(id);
}

std::ostream& operator<<(std::ostream& os, const SLRTable::Action& action) {
  switch (action.type) {
    case SLRTable::SHIFT:
      os << "s" << action.target;
      break;
    case SLRTable::REDUCE:
      os << "r" << action.target;
      break;
    case SLRTable::ACCEPT:
      os << "acc";
      break;
    case SLRTable::ERROR:
      os << "error";
      break;
    default:
      os << "unknown";
      break;
  }
  return os;
}

int SLRTable::find_grammar(const Grammar &grammar) const {
  return grammar_to_id_.at(grammar);
}

void SLRTable::build() {
  build(item_cluster_); // 直接调用有参版
}

int SLRTable::compute_conflict() {
  int conflict_state_count = 0;
  // 检查ACTION表冲突
  for (const auto& [state, action_row] : action_table_) {
    std::set<std::vector<Action>> seen;
    for (const auto& [symbol, actions] : action_row) {
      if (actions.size() > 1) {
        Conflict conflict;
        conflict.state = state;
        conflict.symbol = symbol;
        conflict.actions = actions;
        conflict.type = detect_conflict_type(actions);
        conflicts_.insert(conflict);
      }

      // 转为 vector 并排序
      std::vector<Action> sorted_actions(actions.begin(), actions.end());
      std::sort(sorted_actions.begin(), sorted_actions.end());
    }
    for (const auto& conflict : conflicts_) {
      if (conflict.state == state) {
        conflict_state_count++;
        break; // 一旦找到一个就算这个状态是冲突状态
      }
    }
  }
  return conflict_state_count;
}

void SLRTable::build(const ItemCluster& cluster) {
  item_cluster_ = cluster;

  action_table_.clear();
  goto_table_.clear();
  conflicts_.clear();
  state_to_id_.clear();
  id_to_state_.clear();
  grammar_to_id_.clear();
  id_to_grammar_.clear();
  accept_states_.clear();

  GrammarSet grammar_set = item_cluster_.grammar_set();
  grammar_set.compute_first();
  grammar_set.compute_follow();

  assign_ids(item_cluster_);
  compute_states(item_cluster_);

  int shift_count = 0;
  int reduce_count = 0;
  int accept_count = 0;
  int shift_state_count = 0;
  int reduce_state_count = 0;
  int accept_state_count = 0;
  int conflict_state_count = 0;

  for (const auto& [state_name, state] : item_cluster_.states()) {
    int state_id = state_to_id_.at(state_name);
    bool has_shift = false;
    bool has_reduce = false;
    bool has_accept = false;

    // 处理GOTO表（非终结符）
    for (const auto& [symbol, target_name] : state.goto_table) {
      if (grammar_set.non_terminals().count(symbol)) {
        if (state_to_id_.count(target_name)) {
          int target_id = state_to_id_.at(target_name);
          add_goto(state_id, symbol, target_id);
        } else {
          std::cerr << "[Warning] GOTO目标状态 " << target_name << " 不存在\n";
        }
      }
    }

    std::vector<Item> closure_items;
    for (const auto& [lhs, items] : state.closure.items()) {
      closure_items.insert(closure_items.end(), items.begin(), items.end());
    }

    for (const auto& item : closure_items) {
      if (item.completed()) {
        const Grammar prod = item.to_grammar();
        int prod_id = grammar_to_id_.at(prod);

        if (item.lhs() == grammar_set.start_symbol() + "'") {
          add_action(state_id, "#", ACCEPT, -1);
          accept_count++;
          has_accept = true;
        } else {
          const auto& follow = grammar_set.follow_set().at(item.lhs());
          for (const auto& a : follow) {
            add_action(state_id, a, REDUCE, prod_id);
            reduce_count++;
            has_reduce = true;
          }
        }
      } else {
        const auto& rhs = item.rhs();
        for (size_t i = 0; i < rhs.size(); ++i) {
          if (!rhs[i].empty() && rhs[i][0] == '`') {
            if (i + 1 < rhs.size()) {
              const std::string& next_sym = rhs[i + 1];
              if (grammar_set.terminals().count(next_sym)) {
                if (state.goto_table.count(next_sym)) {
                  const std::string& target_name = state.goto_table.at(next_sym);
                  if (state_to_id_.count(target_name)) {
                    int target_id = state_to_id_.at(target_name);
                    add_action(state_id, next_sym, SHIFT, target_id);
                    shift_count++;
                    has_shift = true;
                  } else {
                    std::cerr << "[Warning] GOTO目标状态 " << target_name << " 不存在\n";
                  }
                }
              }
            }
            break;
          }
        }
      }
    }

    if (has_shift) shift_state_count++;
    if (has_reduce) reduce_state_count++;
    if (has_accept) accept_state_count++;
  }

  conflict_state_count = compute_conflict();

  std::cout << "\n------ SLR表构建完成 ------\n";
  std::cout << "移进状态数: " << shift_state_count << "    移进动作数: " << shift_count << "\n";
  std::cout << "归约状态数: " << reduce_state_count << "    归约动作数: " << reduce_count << "\n";
  std::cout << "接受状态数: " << accept_state_count << "    接受动作数: " << accept_count << "\n";
  std::cout << "冲突状态数: " << conflict_state_count << "    冲突动作数: " << conflicts_.size() << "\n";
  std::cout << "总状态数 (STATES): " << item_cluster_.states().size() << "\n\n";
}

SLRTable::ConflictType SLRTable::detect_conflict_type(const SLRTable::ActionSet& actions) {
  bool has_shift = false, has_reduce = false;
  for (const auto& act : actions) {
    if (act.type == SHIFT) has_shift = true;
    if (act.type == REDUCE) has_reduce = true;
  }

  if (has_shift && has_reduce) return SHIFT_REDUCE;
  if (!has_shift && has_reduce && actions.size() > 1) return REDUCE_REDUCE;
  if (has_shift && actions.size() > 1) return SHIFT_SHIFT;
  return UNKNOWN;
}

void SLRTable::to_csv(const std::string &filename) const {
  Csv csv;

  GrammarSet grammar_set = item_cluster_.grammar_set();
  grammar_set.compute_symbols();

  // 1. 收集所有出现过的符号
  std::unordered_set<std::string> symbols;
  for (const auto& [_, action_row] : action_table_) {
    for (const auto& [symbol, _] : action_row) {
      symbols.insert(symbol);
    }
  }
  for (const auto& [_, goto_row] : goto_table_) {
    for (const auto& [symbol, _] : goto_row) {
      symbols.insert(symbol);
    }
  }
  symbols.insert("#"); // 强制把 # 加入

  // 2. 分开终结符和非终结符
  std::vector<std::string> terminals, non_terminals;
  for (const auto& sym : symbols) {
    if (sym == "#") {
      terminals.push_back(sym); // # 永远是终结符
    } else if (grammar_set.terminals().count(sym)) {
      terminals.push_back(sym);
    } else {
      non_terminals.push_back(sym);
    }
  }
  std::sort(terminals.begin(), terminals.end());
  std::sort(non_terminals.begin(), non_terminals.end());

  // 3. 写标题行
  std::vector<std::string> header = {"State"};
  header.insert(header.end(), terminals.begin(), terminals.end());
  header.insert(header.end(), non_terminals.begin(), non_terminals.end());
  csv.add_row(header);

  // 4. 收集所有状态编号并升序
  std::unordered_set<int> state_set;
  for (const auto& [state, _] : action_table_) state_set.insert(state);
  for (const auto& [state, _] : goto_table_) state_set.insert(state);

  std::vector<int> states(state_set.begin(), state_set.end());
  std::sort(states.begin(), states.end());

  // 5. 填写每一行
  for (int state : states) {
    std::vector<std::string> row;
    row.push_back(std::to_string(state));

    // 填 ACTION 列（终结符）
    for (const auto& sym : terminals) {
      std::string cell;

      if (auto act_it = action_table_.find(state); act_it != action_table_.end()) {
        if (auto sym_it = act_it->second.find(sym); sym_it != act_it->second.end()) {
          const ActionSet& actions = sym_it->second;

          size_t cnt = 0;
          for (const auto& action : actions) {
            if (cnt++ > 0) cell += "/";

            switch (action.type) {
              case SHIFT:  cell += "s" + std::to_string(action.target); break;
              case REDUCE: cell += "r" + std::to_string(action.target); break;
              case ACCEPT: cell += "acc"; break;
              case ERROR:  cell += "error"; break;
            }
          }
        }
      }

      row.push_back(cell);
    }

    // 填 GOTO 列（非终结符）
    for (const auto& sym : non_terminals) {
      std::string cell;

      if (auto goto_it = goto_table_.find(state); goto_it != goto_table_.end()) {
        if (auto sym_it = goto_it->second.find(sym); sym_it != goto_it->second.end()) {
          const GotoSet& gotos = sym_it->second;

          if (!gotos.empty()) {
            cell = std::to_string(*gotos.begin()); // 如果有多个，这里拿第一个（通常GOTO只一个）
          }
        }
      }

      row.push_back(cell);
    }

    csv.add_row(row);
  }

  csv.save(filename);
}

void SLRTable::read_csv(const std::string& filename) {
  action_table_.clear();
  goto_table_.clear();
  conflicts_.clear();
  Csv csv;
  GrammarSet grammar_set = item_cluster_.grammar_set();
  grammar_set.compute_symbols();

  if (!csv.load(filename)) {
    std::cerr << "Failed to load CSV file: " << filename << std::endl;
    return;
  }

  auto rows = csv.rows();
  if (rows.empty()) return;

  const auto& header = rows[0];
  std::vector<std::string> terminals, non_terminals;

  for (size_t i = 1; i < header.size(); ++i) {
    const std::string& symbol = header[i];
    if (grammar_set.terminals().count(symbol) || symbol == "#") {
      terminals.push_back(symbol);
    } else if (grammar_set.non_terminals().count(symbol)) {
      non_terminals.push_back(symbol);
    } else {
      std::cerr << "[Warning] 无法识别的符号（未出现在文法中）: " << symbol << "\n";
    }
  }

  for (size_t r = 1; r < rows.size(); ++r) {
    const auto& row = rows[r];
    if (row.empty()) continue;
    int state = std::stoi(row[0]);

    for (size_t j = 0; j < terminals.size(); ++j) {
      const std::string& symbol = terminals[j];
      if (1 + j >= row.size()) continue;
      const std::string& cell = row[1 + j];
      if (cell.empty()) continue;

      size_t pos = 0;
      while (pos < cell.size()) {
        if (cell.substr(pos, 3) == "acc") {
          add_action(state, symbol, ACCEPT, -1);
          pos += 3;
        } else if (cell[pos] == 's') {
          int num = std::stoi(cell.substr(pos + 1));
          add_action(state, symbol, SHIFT, num);
          while (pos < cell.size() && cell[pos] != '/') ++pos;
          if (pos < cell.size() && cell[pos] == '/') ++pos;
        } else if (cell[pos] == 'r') {
          int num = std::stoi(cell.substr(pos + 1));
          add_action(state, symbol, REDUCE, num);
          while (pos < cell.size() && cell[pos] != '/') ++pos;
          if (pos < cell.size() && cell[pos] == '/') ++pos;
        } else {
          ++pos;
        }
      }
    }

    for (size_t j = 0; j < non_terminals.size(); ++j) {
      const std::string& symbol = non_terminals[j];
      size_t idx = 1 + terminals.size() + j;
      if (idx >= row.size()) continue;
      const std::string& cell = row[idx];
      if (cell.empty()) continue;
      int target = std::stoi(cell);
      add_goto(state, symbol, target);
    }
  }

  for (const auto& [state, row] : action_table_) {
    for (const auto& [symbol, actions] : row) {
      if (actions.size() > 1) {
        Conflict conflict;
        conflict.state = state;
        conflict.symbol = symbol;
        conflict.actions = actions;
        conflict.type = detect_conflict_type(actions);
        conflicts_.insert(conflict);
      }
    }
  }

}

int SLRTable::extract_number(const std::string& name) {
  size_t pos = name.find_last_of(' ');
  if (pos != std::string::npos && pos + 1 < name.size()) {
    return std::stoi(name.substr(pos + 1));
  }
  return -1; // 提取失败
}

void SLRTable::assign_ids(const ItemCluster& cluster) {
  GrammarSet grammar_set = cluster.grammar_set();
  std::vector<std::string> state_names;
  for (const auto& [state_name, _] : cluster.states()) {
    state_names.push_back(state_name);
  }

  // 按编号排序
  std::sort(state_names.begin(), state_names.end(), [](const std::string& a, const std::string& b) {
    return extract_number(a) < extract_number(b);
  });

  int idx = 0;
  for (const auto& name : state_names) {
    state_to_id_[name] = idx;
    id_to_state_[idx] = name;
    idx++;
  }

  int prod_idx = 0;
  for (const auto& [lhs, productions] : grammar_set.grammars()) {
    for (const auto& prod : productions) {
      grammar_to_id_[prod] = prod_idx;
      id_to_grammar_[prod_idx] = prod;
      prod_idx++;
    }
  }

  // 给扩展文法 S' -> S 分配编号
  Grammar augmented_start(grammar_set.start_symbol() + "'", { grammar_set.start_symbol() });
  grammar_to_id_[augmented_start] = prod_idx;
  id_to_grammar_[prod_idx] = augmented_start;

  // 初始化ACTION和GOTO表
  for (const auto& name : state_names) {
    int state_id = state_to_id_.at(name);
    action_table_[state_id] = {};
    goto_table_[state_id] = {};
  }
}

void SLRTable::compute_states(const ItemCluster& cluster) {
  GrammarSet grammar_set = cluster.grammar_set();
  // 1. 设置起始状态
  if (state_to_id_.count("Item Set 0")) {
    start_state_ = state_to_id_.at("Item Set 0");
  } else {
    throw std::runtime_error("No 'Item Set 0' found in ItemCluster.");
  }

  // 2. 获取起始符号
  const std::string& start_symbol = grammar_set.start_symbol();
  const std::string augmented_start = start_symbol + "'";

  // 3. 遍历每个状态
  for (const auto& [state_name, state] : cluster.states()) {
    int state_id = state_to_id_.at(state_name);

    // 看核（kernel）里有没有点在末尾的项目（完成的项目）
    for (const auto& [lhs, items] : state.kernel.items()) {
      for (const auto& item : items) {
        if (item.completed()) {
          accept_states_.insert(state_id);
        }
      }
    }

    // 再看闭包（closure）里有没有最终接受（S'→S·）
    for (const auto& [lhs, items] : state.closure.items()) {
      for (const auto& item : items) {
        if (item.completed() && lhs == augmented_start) {
          final_accept_state_ = state_id;
        }
      }
    }
  }

  if (accept_states_.empty()) {
    throw std::runtime_error("No accept states found in ItemCluster.");
  }
}
