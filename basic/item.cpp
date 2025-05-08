#include "item.hpp"
#include "utils/format.hpp"

// ===== Item 类实现 =====

Item::Item() = default;

Item::Item(const Grammar &grammar, size_t pos)
  : lhs_(grammar.lhs()), rhs_(insert_dot(grammar.rhs(), pos)) {}

const std::string &Item::lhs() const { return lhs_; }

const std::vector<std::string> &Item::rhs() const { return rhs_; }

bool Item::completed() const {
  if (rhs_.empty()) return false;
  // 遍历找到最后一个带点的符号
  for (auto it = rhs_.rbegin(); it != rhs_.rend(); ++it) {
    if (!it->empty() && (*it)[0] == '`') {
      return (it == rhs_.rbegin());
    }
  }
  return false;
}

Grammar Item::to_grammar() const {
  std::vector<std::string> clean_rhs;
  for (const auto& symbol : rhs_) {
    if (symbol != "`") {
      clean_rhs.push_back(symbol);
    }
  }
  if (clean_rhs.empty()) {
    clean_rhs.push_back("ε"); // 如果右部是空，加上"ε"
  }
  return Grammar(lhs_, clean_rhs);
}

json Item::to_json() const {
  return rhs_;
}

bool Item::operator==(const Item &other) const {
  return lhs_ == other.lhs_ && rhs_ == other.rhs_;
}

std::ostream &operator<<(std::ostream &os, const Item &item) {
  os << item.lhs_ << " -> ";
  for (size_t i = 0; i < item.rhs_.size(); ++i) {
    if (item.rhs_[i] == "`") {
      os << "\u2022";
    } else {
      os << item.rhs_[i];
    }
    if (i != item.rhs_.size() - 1) {
      os << " ";
    }
  }
  return os;
}

std::vector<std::string> Item::insert_dot(const std::vector<std::string> &rhs, size_t pos) {
  std::vector<std::string> result;

  // 如果rhs只包含一个"ε"
  if (rhs.size() == 1 && rhs[0] == "ε") {
    if (pos == 0) {
      result.push_back("`"); // 只放一个点
    }
    // pos > 0 不放任何东西，result保持空
    return result;
  }

  // 普通情况
  result.reserve(rhs.size() + 1);
  for (size_t i = 0; i <= rhs.size(); ++i) {
    if (i == pos) {
      result.push_back("`");
    }
    if (i < rhs.size()) {
      result.push_back(rhs[i]);
    }
  }
  return result;
}

// ===== ItemSet 类实现 =====

// 默认构造
ItemSet::ItemSet() = default;

ItemSet::ItemSet(const Grammar& grammar, size_t pos) {
  add(grammar, pos);
}

ItemSet::ItemSet(const Item& item) {
  add(item);
}

ItemSet::ItemSet(const GrammarSet& grammar_set, size_t pos) {
  for (const auto& [lhs, grammar_list] : grammar_set.grammars()) {
    for (const auto& grammar : grammar_list) {
      add(grammar, pos);
    }
  }
}

void ItemSet::add(const Item &item) {
  items_[item.lhs()].push_back(item);
}

void ItemSet::add(const Grammar &grammar, size_t pos) {
  Item item(grammar, pos);
  add(item);
}

const std::unordered_map<std::string, std::vector<Item>> &ItemSet::items() const {
  return items_;
}

std::vector<Item> &ItemSet::operator[](const std::string &lhs) {
  return items_[lhs];
}

const std::vector<Item> &ItemSet::operator[](const std::string &lhs) const {
  auto it = items_.find(lhs);
  if (it == items_.end()) {
    static const std::vector<Item> empty;
    return empty;
  }
  return it->second;
}

bool ItemSet::contains(const Item &item) const {
  // items_为哈希表, 以O(1)复杂度搜索item的左部
  auto it = items_.find(item.lhs());
  if (it == items_.end()) {
    return false;
  }
  // 依次判断item的右部是否在项目集中
  const auto &item_list = it->second;
  for (const auto &existing_item : item_list) {
    if (existing_item == item) {
      return true;
    }
  }
  return false;
}

bool ItemSet::empty() const {
  for (const auto& [lhs, item_list] : items_) {
    if (!item_list.empty()) {
      return false;
    }
  }
  return true;
}

ItemSet ItemSet::move() const {
  ItemSet moved_set;
  for (const auto& [lhs, item_list] : items_) {
    for (const auto& item : item_list) {
      const auto& rhs = item.rhs();
      auto it = std::find(rhs.begin(), rhs.end(), "`");
      if (it != rhs.end() && (it + 1) != rhs.end()) {
        size_t dot_pos = std::distance(rhs.begin(), it);

        std::vector<std::string> real_rhs;
        for (const auto& sym : rhs) {
          if (sym != "`") {
            real_rhs.push_back(sym);
          }
        }
        Item moved_item(Grammar(item.lhs(), real_rhs), dot_pos + 1);
        moved_set.add(moved_item);
      }
    }
  }
  return moved_set;
}

ItemSet ItemSet::move(const std::string& symbol) const {
  ItemSet moved_set;
  for (const auto& [lhs, item_list] : items_) {
    for (const auto& item : item_list) {
      const auto& rhs = item.rhs();
      auto it = std::find(rhs.begin(), rhs.end(), "`");
      if (it != rhs.end() && (it + 1) != rhs.end() && *(it + 1) == symbol) {
        size_t dot_pos = std::distance(rhs.begin(), it);

        std::vector<std::string> real_rhs;
        for (const auto& sym : rhs) {
          if (sym != "`") {
            real_rhs.push_back(sym);
          }
        }
        Item moved_item(Grammar(item.lhs(), real_rhs), dot_pos + 1);
        moved_set.add(moved_item);
      }
    }
  }
  return moved_set;
}

std::unordered_set<std::string> ItemSet::next_symbols() const {
  std::unordered_set<std::string> symbols;
  for (const auto& [lhs, item_list] : items_) {
    for (const auto& item : item_list) {
      const auto& rhs = item.rhs();
      auto it = std::find(rhs.begin(), rhs.end(), "`");
      if (it != rhs.end() && (it + 1) != rhs.end()) {
        symbols.insert(*(it + 1));
      }
    }
  }
  return symbols;
}

ItemSet ItemSet::closure(const GrammarSet &grammar_set) const {
  return closure(*this, grammar_set);
}

ItemSet ItemSet::closure(const Item &item, const GrammarSet &grammar_set) {
  ItemSet closure_set;
  std::stack<Item> stk;

  closure_set.add(item);
  stk.push(item);

  while (!stk.empty()) {
    Item current_item = stk.top();
    stk.pop();

    const auto &rhs = current_item.rhs();
    auto it = std::find(rhs.begin(), rhs.end(), "`");

    if (it != rhs.end() && (it + 1) != rhs.end()) {
      std::string next_symbol = *(it + 1);

      if (grammar_set.grammars().count(next_symbol)) {
        const auto &production_list = grammar_set.grammars().at(next_symbol);
        for (const auto &prod : production_list) {
          Item new_item(prod, 0);
          if (!closure_set.contains(new_item)) {
            closure_set.add(new_item);
            stk.push(new_item);
          }
        }
      }
    }
  }

  return closure_set;
}

ItemSet ItemSet::closure(const ItemSet &item_set, const GrammarSet &grammar_set) {
  ItemSet closure_set;
  // 维护一个栈, 以实现DFS
  std::stack<Item> stk;

  // 将待求闭包的项目集中的所有项目加入闭包集
  // 且加入待搜索栈 stk
  for (const auto &[lhs, item_list] : item_set.items_) {
    for (const auto &item : item_list) {
      closure_set.add(item);
      stk.push(item);
    }
  }

  while (!stk.empty()) {
    Item current_item = stk.top();
    stk.pop();

    // 文法的右部
    const auto &rhs = current_item.rhs();
    // 找到点的位置
    auto it = std::find(rhs.begin(), rhs.end(), "`");

    // 如果点后还有符号
    if (it != rhs.end() && (it + 1) != rhs.end()) {
      std::string next_symbol = *(it + 1);

      // 如果这个符号有产生式
      if (grammar_set.grammars().count(next_symbol)) {
        const auto &production_list = grammar_set.grammars().at(next_symbol);
        for (const auto &prod : production_list) {
          // 根据产生式, 构造一个点在最左边位置的项目
          Item new_item(prod, 0);
          // 如果结果闭包集里不包含这个项目, 则加入闭包集和 stk
          if (!closure_set.contains(new_item)) {
            closure_set.add(new_item);
            stk.push(new_item);
          }
        }
      }
    }
  }

  return closure_set;
}

void ItemSet::union_set(const ItemSet &other) {
  for (const auto &[lhs, item_list] : other.items_) {
    for (const auto &item : item_list) {
      if (!contains(item)) {
        add(item);
      }
    }
  }
}

json ItemSet::to_json() const {
  json result;
  for (const auto &[lhs, item_list] : items_) {
    json productions = json::array();
    for (const auto &item : item_list) {
      productions.push_back(item.to_json());
    }
    result[lhs] = productions;
  }
  return result;
}

json ItemSet::to_json(const std::string &filename) const {
  json result = to_json();
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file to save json: " + filename);
  }
  compact_serializer::dump(result, file, true, 2);
  return result;
}

std::ostream &operator<<(std::ostream &os, const ItemSet &item_set) {
  json j = item_set.to_json();
  compact_serializer::dump(j, os, true, 2);
  return os;
}

bool ItemSet::operator==(const ItemSet& other) const {
  if (items_.size() != other.items_.size()) {
    return false;
  }

  for (const auto& [lhs, item_list] : items_) {
    auto it = other.items_.find(lhs);
    if (it == other.items_.end()) {
      return false;
    }

    const auto& other_list = it->second;
    if (item_list.size() != other_list.size()) {
      return false;
    }

    for (size_t i = 0; i < item_list.size(); ++i) {
      if (!(item_list[i] == other_list[i])) {
        return false;
      }
    }
  }

  return true;
}

// ===== ItemCluster 类实现 =====

ItemCluster::ItemCluster(const GrammarSet& grammar_set)
  : grammar_set_(grammar_set), state_counter_(0) {}

std::string ItemCluster::add(const ItemSet& kernel) {
  std::string name = generate_state_name(); // 自动生成新的名字，比如"Item Set 5"

  State new_state;
  new_state.kernel = kernel;
  new_state.closure = kernel.closure(grammar_set_);

  states_[name] = new_state;

  return name;
}

std::string ItemCluster::find(const ItemSet& item_set) const {
  for (const auto& [name, state] : states_) {
    if (state.closure == item_set) {
      return name;
    }
  }
  return "";
}

const GrammarSet & ItemCluster::grammar_set() const {
  return grammar_set_;
}

const std::unordered_map<std::string, ItemCluster::State>& ItemCluster::states() const {
  return states_;
}

ItemCluster::State& ItemCluster::operator[](const std::string& state_name) {
  return states_[state_name];
}

const ItemCluster::State& ItemCluster::operator[](const std::string& state_name) const {
  auto it = states_.find(state_name);
  if (it == states_.end()) {
    throw std::out_of_range("State '" + state_name + "' not found in ItemCluster.");
  }
  return it->second;
}

void ItemCluster::parse_stream(const std::string& content) {
  json j;
  try {
    j = json::parse(content);
  } catch (const std::exception& e) {
    std::cerr << "JSON parsing error: " << e.what() << std::endl;
    return;
  }

  states_.clear();
  state_counter_ = 0;

  for (const auto& [state_name, state_data] : j.items()) {
    State state;

    if (state_data.contains("Kernel")) {
      for (const auto& [lhs, rhs_list] : state_data["Kernel"].items()) {
        for (const auto& rhs : rhs_list) {
          Grammar g(lhs, rhs.get<std::vector<std::string>>());
          state.kernel.add(g, 0);
        }
      }
    }

    if (state_data.contains("Closure")) {
      for (const auto& [lhs, rhs_list] : state_data["Closure"].items()) {
        for (const auto& rhs : rhs_list) {
          Grammar g(lhs, rhs.get<std::vector<std::string>>());
          state.closure.add(g, 0);
        }
      }
    }

    if (state_data.contains("Goto")) {
      for (const auto& [symbol, target] : state_data["Goto"].items()) {
        state.goto_table[symbol] = target.get<std::string>();
      }
    }

    states_[state_name] = state;

    if (state_name.find("Item Set ") == 0) {
      int num = std::stoi(state_name.substr(9));
      if (num >= state_counter_) {
        state_counter_ = num + 1;
      }
    }
  }
}

std::string ItemCluster::generate_state_name() {
  return "Item Set " + std::to_string(state_counter_++);
}

void ItemCluster::set_goto(const std::string& from_state, const std::string& symbol, const std::string& to_state) {
  states_.at(from_state).goto_table[symbol] = to_state;
}

std::ostream& operator<<(std::ostream& os, const ItemCluster& cluster) {
  json j = cluster.to_json();
  compact_serializer::dump(j, os, true, 2);
  return os;
}

void ItemCluster::parse(const std::string& content) {
  parse_stream(content);
}

void ItemCluster::parse_file(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();
  parse_stream(buffer.str());
}

void ItemCluster::build() {
  // 创建初始项目 [S' -> • S]
  std::string start_symbol = grammar_set_.start_symbol();
  Grammar start_grammar(start_symbol + "'", {start_symbol});
  Item start_item(start_grammar, 0);

  // 清空现有状态
  states_.clear();
  state_counter_ = 0;

  // 添加初始项
  add(start_item);

  std::unordered_set<std::string> processed;

  bool changed = true;
  while (changed) {
    changed = false;

    // 注意：复制一份当前 states 列表
    auto current_states = states_;

    for (const auto& [state_name, state] : current_states) {
      if (processed.count(state_name)) continue; // 跳过已处理

      processed.insert(state_name); // 标记为已处理

      // 遍历 closure 中每个 next symbol
      for (const auto& symbol : state.closure.next_symbols()) {
        ItemSet moved_kernel = state.closure.move(symbol);
        if (moved_kernel.empty()) continue;

        ItemSet closure_set = moved_kernel.closure(grammar_set_);

        std::string existing = find(closure_set);
        std::string target_name;

        if (existing.empty()) {
          target_name = add(moved_kernel);
        } else {
          target_name = existing;
        }

        set_goto(state_name, symbol, target_name);

        changed = true;
      }
    }
  }
}

json ItemCluster::to_json() const {
  json cluster_json;

  std::map<std::string, State> ordered_states(states_.begin(), states_.end());

  for (const auto& [name, state] : ordered_states) {
    json state_json;

    // ！！顺序一定要自己控制
    state_json["Kernel"] = state.kernel.to_json();
    state_json["Closure"] = state.closure.to_json();

    // Goto
    std::map<std::string, std::string> ordered_goto(state.goto_table.begin(), state.goto_table.end());
    state_json["Goto"] = ordered_goto;

    cluster_json[name] = state_json;
  }

  return cluster_json;
}

void ItemCluster::to_json(const std::string &filename) const {
  json j = to_json();

  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file to save json: " + filename);
  }

  compact_serializer::dump(j, file, true, 2);
}

std::string ItemCluster::to_txt() const {
  std::ostringstream oss;

  // 保证按顺序输出
  std::map<std::string, State> ordered_states(states_.begin(), states_.end());

  for (const auto& [state_name, state] : ordered_states) {
    oss << state_name << ":\n";

    // Kernel
    oss << "  Kernel:\n";
    for (const auto& [lhs, item_list] : state.kernel.items()) {
      for (const auto& item : item_list) {
        oss << "    " << item.lhs() << " ->";

        for (const auto& sym : item.rhs()) {
          if (sym == "`") {
            oss << " \u2022"; // 实际UTF-8的 • 点
          } else {
            oss << " " << sym;
          }
        }

        oss << "\n";
      }
    }

    // Closure
    oss << "  Closure:\n";
    for (const auto& [lhs, item_list] : state.closure.items()) {
      for (const auto& item : item_list) {
        oss << "    " << item.lhs() << " ->";

        for (const auto& sym : item.rhs()) {
          if (sym == "`") {
            oss << " \u2022"; // 点
          } else {
            oss << " " << sym;
          }
        }

        oss << "\n";
      }
    }

    // Goto
    oss << "  Goto:\n";
    std::map<std::string, std::string> ordered_goto(state.goto_table.begin(), state.goto_table.end());
    for (const auto& [symbol, target_state] : ordered_goto) {
      oss << "    " << symbol << " -> " << target_state << "\n";
    }
  }

  return oss.str();
}

void ItemCluster::to_txt(const std::string &filename) const {
  std::string content = to_txt();

  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file to save txt: " + filename);
  }

  file << content;
}

void ItemCluster::to_dot(const std::string& filename) const {
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file to save dot: " + filename);
  }

  file << "digraph ItemCluster {\n";
  file << "  rankdir=LR;\n"; // 左到右布局
  file << "  node [shape=box];\n"; // 节点用矩形框

  // 写所有状态节点（带Closure内容）
  for (const auto& [state_name, state] : states_) {
    std::ostringstream label;

    label << state_name << "\\n"; // 第一行是Item Set名字

    for (const auto& [lhs, item_list] : state.closure.items()) {
      for (const auto& item : item_list) {
        label << lhs << " ->";
        for (const auto& sym : item.rhs()) {
          if (sym == "`")
            label << " \u2022"; // 点 •
          else
            label << " " << sym;
        }
        label << "\\n"; // 换行（Graphviz里是 \\n，不是\n）
      }
    }

    // 写节点，带label
    file << "  \"" << state_name << "\" [label=\"" << label.str() << "\"];\n";
  }

  // 写所有转移边
  for (const auto& [state_name, state] : states_) {
    for (const auto& [symbol, target_name] : state.goto_table) {
      file << "  \"" << state_name << "\" -> \"" << target_name
           << "\" [label=\"" << symbol << "\"];\n";
    }
  }

  file << "}\n";
}
