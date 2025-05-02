#include "grammar.hpp"

#include "item.hpp"
#include "utils/format.hpp"
#include "utils/strtool.hpp" // 引入新的字符串工具

using namespace strtool; // 省得每次手动写 strtool::trim

// Grammar 实现

Grammar::Grammar() = default;

Grammar::Grammar(const std::string& grammar_text) {
  auto arrow_pos = grammar_text.find("->");
  if (arrow_pos == std::string::npos) {
    throw std::runtime_error("Invalid grammar format: missing ->");
  }

  lhs_ = trim(grammar_text.substr(0, arrow_pos));
  std::string rhs_all = trim(grammar_text.substr(arrow_pos + 2));

  // 判断是不是 ε
  // if (rhs_all == "ε") {
  //   rhs_.clear(); // 空产生式
  // } else {
  //   rhs_ = split(rhs_all, ' ');
  // }
  rhs_ = split(rhs_all, ' ');
}

Grammar::Grammar(const std::string& lhs, const std::vector<std::string>& rhs)
  : lhs_(lhs), rhs_(rhs) {}

const std::string& Grammar::lhs() const {
  return lhs_;
}

const std::vector<std::string>& Grammar::rhs() const {
  return rhs_;
}

json Grammar::to_json() const {
  return rhs_;
}

bool Grammar::operator==(const std::string &other) const {
  const Grammar temp = Grammar(other);
  return *this == temp;
}

bool Grammar::operator==(const Grammar& other) const {
  return lhs_ == other.lhs_ && rhs_ == other.rhs_;
}

std::ostream& operator<<(std::ostream& os, const Grammar& grammar) {
  os << grammar.lhs_ << " -> ";
  for (size_t i = 0; i < grammar.rhs_.size(); ++i) {
    os << grammar.rhs_[i];
    if (i != grammar.rhs_.size() - 1) {
      os << " ";
    }
  }
  return os;
}

// GrammarSet 实现

GrammarSet::GrammarSet() : start_("") {}

GrammarSet::GrammarSet(const std::string& source) : start_("") {
  if (source.find("->") != std::string::npos) {
    parse(source);
  } else {
    parse_file(source);
  }
}

GrammarSet::GrammarSet(const std::string& source, const std::string& start_symbol)
  : start_(start_symbol) {
  if (source.find("->") != std::string::npos) {
    parse(source);
  } else {
    parse_file(source);
  }
}

void GrammarSet::add(const Grammar& grammar) {
  grammars_[grammar.lhs()].push_back(grammar);
}

void GrammarSet::add(const std::string& grammar_text) {
  Grammar grammar(grammar_text);
  add(grammar);
}

bool GrammarSet::parse(const std::string& grammar_text) {
  std::istringstream iss(grammar_text);
  return parse_stream(iss);
}

bool GrammarSet::parse_file(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Cannot open grammar file: " << filename << std::endl;
    return false;
  }
  return parse_stream(file);
}

bool GrammarSet::parse(const std::string& grammar_text, const std::string& start_symbol) {
  start_ = start_symbol;
  return parse(grammar_text);
}

bool GrammarSet::parse_file(const std::string& filename, const std::string& start_symbol) {
  start_ = start_symbol;
  return parse_file(filename);
}

const std::unordered_map<std::string, std::vector<Grammar>>& GrammarSet::grammars() const {
  return grammars_;
}

std::vector<Grammar>& GrammarSet::operator[](const std::string& lhs) {
  return grammars_[lhs];
}

const std::vector<Grammar>& GrammarSet::operator[](const std::string& lhs) const {
  auto it = grammars_.find(lhs);
  if (it == grammars_.end()) {
    static const std::vector<Grammar> empty;
    return empty;
  }
  return it->second;
}

void GrammarSet::compute_symbols() {
  terminals_.clear();
  non_terminals_.clear();

  // 先收集所有左部 -> 非终结符
  for (const auto& [lhs, productions] : grammars_) {
    non_terminals_.insert(lhs);
  }

  // 再扫描所有右部
  for (const auto& [lhs, productions] : grammars_) {
    for (const auto& prod : productions) {
      const auto& rhs = prod.rhs();
      for (const auto& symbol : rhs) {
        // 如果不是非终结符，就认定是终结符
        if (non_terminals_.find(symbol) == non_terminals_.end() && !symbol.empty()) {
          terminals_.insert(symbol);
        }
      }
    }
  }
}

std::unordered_set<std::string> GrammarSet::compute_first(const std::string& symbol) {
  if (first_.count(symbol)) {
    return first_[symbol];
  }

  if (in_progress_.count(symbol)) {
    // 正在计算，检测到递归，返回空集合防止死循环
    return {};
  }

  in_progress_.insert(symbol); // 标记正在计算

  std::unordered_set<std::string> result;

  // 终结符：FIRST就是自己
  if (terminals_.count(symbol)) {
    result.insert(symbol);
    first_[symbol] = result;
    in_progress_.erase(symbol);
    return result;
  }

  // 非终结符
  if (!grammars_.count(symbol)) {
    // 防止万一symbol没定义
    in_progress_.erase(symbol);
    return {};
  }

  for (const auto& prod : grammars_.at(symbol)) {
    const auto& rhs = prod.rhs();

    if (rhs.empty() || (rhs.size() == 1 && rhs[0] == "ε")) {
      // 空产生式
      result.insert("ε");
      continue;
    }

    bool all_nullable = true;
    for (const auto& sym : rhs) {
      auto sym_first = compute_first(sym);

      for (const auto& f : sym_first) {
        if (f != "ε") {
          result.insert(f);
        }
      }

      if (sym_first.find("ε") == sym_first.end()) {
        all_nullable = false;
        break;
      }
    }

    if (all_nullable) {
      result.insert("ε");
    }
  }

  first_[symbol] = result;
  in_progress_.erase(symbol);
  return result;
}

std::unordered_map<std::string, std::unordered_set<std::string>> GrammarSet::compute_first() {
  first_.clear();
  compute_symbols();

  // 终结符：FIRST集合就是自己
  for (const auto& t : terminals_) {
    first_[t].insert(t);
  }

  // 非终结符：递归计算
  for (const auto& nt : non_terminals_) {
    compute_first(nt);
  }

  return first_;
}

std::unordered_map<std::string, std::unordered_set<std::string>> GrammarSet::compute_follow() {
  follow_.clear();
  compute_symbols(); // 先确保 terminals_ 和 non_terminals_ 正确
  compute_first();   // 确保 first_ 表正确

  // 起始符号的 FOLLOW 集必须包含 #
  follow_[start_].insert("#");

  bool changed;
  do {
    changed = false;
    for (const auto& [lhs, productions] : grammars_) {
      for (const auto& prod : productions) {
        const auto& rhs = prod.rhs();

        // 遍历产生式右部的每个符号
        for (size_t i = 0; i < rhs.size(); ++i) {
          const std::string& B = rhs[i];

          // 只处理非终结符
          if (non_terminals_.find(B) == non_terminals_.end()) continue;

          bool nullable = true; // 记录右侧后续符号是否可推出ε

          // 从 B 后面的每一个符号开始处理
          for (size_t j = i + 1; j < rhs.size(); ++j) {
            const auto& sym = rhs[j];

            // 将 sym 的 FIRST 集中的非ε元素加入 FOLLOW(B)
            for (const auto& f : first_[sym]) {
              if (f != "ε" && follow_[B].insert(f).second) {
                changed = true;
              }
            }

            // 如果 sym 的 FIRST 集中不包含 ε，停止向后传递
            if (first_[sym].find("ε") == first_[sym].end()) {
              nullable = false;
              break;
            }
          }

          // 如果B后面所有符号都能推出ε，或者B已经是最后一个符号
          // 把 FOLLOW(lhs) 加到 FOLLOW(B)
          if (nullable || (i + 1 == rhs.size())) {
            for (const auto& f : follow_[lhs]) {
              if (follow_[B].insert(f).second) {
                changed = true;
              }
            }
          }
        }
      }
    }
  } while (changed); // 如果有变化，继续迭代

  return follow_;
}

void GrammarSet::set_start(const std::string& start_symbol) {
  start_ = start_symbol;
}

json GrammarSet::to_json() const {
  json result;
  for (const auto& [lhs, grammar_list] : grammars_) {
    json productions = json::array();
    for (const auto& grammar : grammar_list) {
      productions.push_back(grammar.to_json());
    }
    result[lhs] = productions;
  }
  return result;
}

json GrammarSet::to_json(const std::string& filename) const {
  json result = to_json();
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file to save json: " + filename);
  }
  compact_serializer::dump(result, file, true, 2);
  return result;
}

std::ostream& operator<<(std::ostream& os, const GrammarSet& grammar_set) {
  json j = grammar_set.to_json();
  compact_serializer::dump(j, os, true, 2);
  return os;
}

bool GrammarSet::parse_stream(std::istream& input) {
  std::string line;
  bool first_production = true;

  while (std::getline(input, line)) {
    line = trim(line);
    if (line.empty()) continue;

    auto arrow_pos = line.find("->");
    if (arrow_pos == std::string::npos) {
      std::cerr << "Invalid grammar line: " << line << " (missing ->)" << std::endl;
      return false;
    }

    std::string lhs = trim(line.substr(0, arrow_pos));
    std::string rhs_all = trim(line.substr(arrow_pos + 2));
    std::vector<std::string> symbols = split(rhs_all, ' ');

    if (first_production && start_.empty()) {
      start_ = lhs; // 第一次产生式设置 start_
    }
    first_production = false;

    std::vector<std::string> current_rhs;
    for (const auto& sym : symbols) {
      if (sym == "|") {
        if (!current_rhs.empty()) {
          if (current_rhs.size() == 1 && current_rhs[0] == "ε") {
            grammars_[lhs].emplace_back(lhs, std::vector<std::string>{"ε"});
          } else {
            std::vector<std::string> filtered_rhs;
            for (const auto& s : current_rhs) {
              if (s != "ε") filtered_rhs.push_back(s);
            }
            if (!filtered_rhs.empty()) {
              grammars_[lhs].emplace_back(lhs, filtered_rhs);
            }
          }
          current_rhs.clear();
        }
      } else {
        current_rhs.push_back(sym);
      }
    }

    // 处理最后一部分
    if (!current_rhs.empty()) {
      if (current_rhs.size() == 1 && current_rhs[0] == "ε") {
        grammars_[lhs].emplace_back(lhs, std::vector<std::string>{"ε"});
      } else {
        std::vector<std::string> filtered_rhs;
        for (const auto& s : current_rhs) {
          if (s != "ε") filtered_rhs.push_back(s);
        }
        if (!filtered_rhs.empty()) {
          grammars_[lhs].emplace_back(lhs, filtered_rhs);
        }
      }
    }
  }

  return true;
}





