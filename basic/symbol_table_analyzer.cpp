//
// Created by WangLele on 25-5-2.
//

#include "symbol_table_analyzer.hpp"

void SymbolTableAnalyzer::SymbolTable::add_entry(const std::shared_ptr<Entry> &entry) {
  // 检查是否重名
  for (const auto& e : entries) {
    if (e->name == entry->name) {
      throw std::runtime_error("Symbol '" + entry->name + "' is already defined in this scope.");
    }
  }
  entries.push_back(entry);
}

std::ostream &operator<<(std::ostream &os, const SymbolTableAnalyzer::Entry &entry) {
  os << "(name: " << entry.name << ", type: " << entry.type
     << ", offset: " << entry.offset << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os, const SymbolTableAnalyzer::ArrayEntry &entry) {
  os << "(name: " << entry.name << ", type: " << entry.type
     << ", offset: " << entry.offset << ", etype: " << entry.etype
     << ", base: " << entry.base << ", dims: " << entry.dims
     << ", dim: [";
  for (const auto &d : entry.dim) {
    os << d << " ";
  }
  os << "])";
  return os;
}

std::ostream &operator<<(std::ostream &os, const SymbolTableAnalyzer::FuncEntry &entry) {
  os << "(name: " << entry.name << ", type: " << entry.type
     << ", offset: " << entry.offset << ", rtype: " << entry.rtype
     << ", mytab: " << entry.mytab->name << ")";
  return os;
}

std::ostream & operator<<(std::ostream &os, const SymbolTableAnalyzer::SymbolTable &st) {
  os << (SymbolTableAnalyzer::get_table_name(st)) << ": {" << std::endl;
  os << "  width: " << st.width
     << "  argc: " << st.argc
     << "  rtype: " << st.rtype
     << "  level: " << st.level
     << std::endl;
  os << "  arglist: ";
  for (const auto& arg : st.arglist) {
    os << arg << " ";
  }
  os << std::endl;
  os << "  entries: {" << std::endl;
  for (const auto& entry : st.entries) {
    os << "    " << *entry << std::endl;
  }
  os << "  }" << std::endl;
  os << "}" << std::endl;
  return os;
}

std::ostream & operator<<(std::ostream &os, const SymbolTableAnalyzer::Process &p) {
  os << "(" << p.state << ", " << p.token << ", " << p.action << ")";
  return os;
}

SymbolTableAnalyzer::SymbolTableAnalyzer(const SLRTable& slr_table)
  : slr_table_(slr_table) {

}

bool SymbolTableAnalyzer::parse(const std::vector<Lexical::Token>& tokens) {
  return analyze_tokens(tokens);
}

bool SymbolTableAnalyzer::parse_file(const std::string& filename, Lexical lexical) {
  const std::vector<Lexical::Token> tokens = lexical.analyze(filename);
  return analyze_tokens(tokens);
}

void SymbolTableAnalyzer::to_txt(const std::string &filename) const {
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "[错误] 无法打开文件 " << filename << std::endl;
    return;
  }
  file << *this;
}

const std::string & SymbolTableAnalyzer::get_name(const Lexical::Token& token) {
  return token.lexeme;
}

bool SymbolTableAnalyzer::analyze_tokens(const std::vector<Lexical::Token>& tokens) {
  processes_.clear();

  // 添加结束符 token
  std::vector<Lexical::Token> tok = tokens;
  tok.push_back({ "#", "#" });

  std::stack<int> state_stack;
  std::stack<Lexical::Token> stack_token;

  state_stack.push(slr_table_.start_state());

  size_t pos = 0;

  while (true) {
    // 跳过 NEWLINE
    while (pos < tok.size() && tok[pos].type == "NEWLINE") {
      ++pos;
    }

    int current_state = state_stack.top();
    std::string symbol = tok[pos].type;

    const auto* action_set = slr_table_.get_action(current_state, symbol);

    if (!action_set || action_set->empty()) {
      std::cerr << "[错误] 状态 " << current_state << " 符号 '" << symbol << "' 无有效动作。\n";
      return false;
    }

    if (action_set->size() > 1) {
      std::cerr << "[冲突] 状态 " << current_state << " 符号 '" << symbol << "' 有多个动作，冲突如下：\n";
      for (const auto& act : *action_set) {
        std::cerr << "  -> " << act << "\n";
      }
      return false;
    }

    const auto& action = *action_set->begin();
    processes_.push_back({ current_state, tok[pos], action });

    if (action.type == SLRTable::SHIFT) {
      state_stack.push(action.target);
      stack_token.push(tok[pos]);

      if (!shift(action.target, tok[pos])) {
        std::cerr << "[错误] 子类 shift(" << action.target << ") 执行失败。\n";
        return false;
      }

      ++pos;
    }
    else if (action.type == SLRTable::REDUCE) {
      const Grammar& g = slr_table_.find_grammar(action.target);
      const auto& rhs = g.rhs();

      bool is_epsilon = (rhs.empty() || (rhs.size() == 1 && rhs[0] == "ε"));

      std::vector<Lexical::Token> reduced_tokens;

      if (!is_epsilon) {
        for (size_t i = 0; i < rhs.size(); ++i) {
          if (!state_stack.empty()) state_stack.pop();
          if (!stack_token.empty()) {
            reduced_tokens.push_back(stack_token.top());
            stack_token.pop();
          }
        }
        std::reverse(reduced_tokens.begin(), reduced_tokens.end()); // 保持原顺序
      }

      if (state_stack.empty()) {
        std::cerr << "[错误] 归约时状态栈为空。\n";
        return false;
      }

      int top_state = state_stack.top();
      const auto* goto_set = slr_table_.get_goto(top_state, g.lhs());

      if (!goto_set || goto_set->empty()) {
        std::cerr << "[错误] GOTO(" << top_state << ", " << g.lhs() << ") 无目标状态。\n";
        return false;
      }

      state_stack.push(*goto_set->begin());

      // 规约后的 LHS token 入栈
      stack_token.push({ g.lhs(), g.lhs() });

      if (!reduce(action.target, reduced_tokens)) {
        std::cerr << "[错误] 子类 reduce(" << action.target << ") 执行失败。\n";
        return false;
      }
    }
    else if (action.type == SLRTable::ACCEPT) {
      std::cout << "[信息] 输入分析成功，状态 " << current_state << " 接受。\n";
      return true;
    }
    else {
      std::cerr << "[错误] 未知动作类型。\n";
      return false;
    }
  }
}

std::ostream& operator<<(std::ostream& os, const SymbolTableAnalyzer& analyzer) {
  for (const auto& process : analyzer.processes_) {
    os << process << "\n";
  }
  return os;
}

SymbolTableAnalyzer1::SymbolTableAnalyzer1(const SLRTable& slr_table) : SymbolTableAnalyzer(slr_table) {
  std::shared_ptr<SymbolTable> system_table = std::make_shared<SymbolTable>("system_table");
  push_symbol_table(system_table);
  map_symbol_table_["system_table"] = system_table;
}

std::unordered_map<std::string, SymbolTableAnalyzer::TablePtr> SymbolTableAnalyzer1::symbol_table() const {
  return map_symbol_table_;
}

// 根据不同的属性方程, 由程序员实现函数的定义, 父类提供接口.
// 否则, 对属性方程过于抽象, 不容易实现.

bool SymbolTableAnalyzer1::shift(int state_id, const Lexical::Token &token) {
  return true;
}

bool SymbolTableAnalyzer1::reduce_var_declaration(const std::vector<Lexical::Token>& tokens) {
  // std::cout << "04: size: " << tokens.size() << "\n";
  return true;
}

bool SymbolTableAnalyzer1::reduce_func_declaration(const std::vector<Lexical::Token>& tokens) {
  // D -> T d ( A' ) { D' S' }
  //      0 1 2 3  4 5 6  7  8
  TablePtr symbol_table = pop_symbol_table();
  TablePtr outer = stack_symbol_table_.top();
  symbol_table->name = get_name(tokens[1]);
  symbol_table->outer = outer;
  map_symbol_table_[get_table_name(*symbol_table)] = symbol_table;
  // 创建函数登记项
  FuncEntryPtr entry = std::make_shared<FuncEntry>();
  entry->name = get_name(tokens[1]);
  entry->type = "func";
  entry->rtype = get_name(tokens[0]);
  entry->mytab = symbol_table;
  outer->width += 8;
  entry->offset = outer->width;
  outer->add_entry(entry);
  return true;
}

bool SymbolTableAnalyzer1::reduce_func_params(const std::vector<Lexical::Token>& tokens) {
  std::shared_ptr<SymbolTable> symbol_table = std::make_shared<SymbolTable>();
  push_symbol_table(symbol_table);
  return true;
}

bool SymbolTableAnalyzer1::reduce(int grammar_id, const std::vector<Lexical::Token>& tokens) {
  Grammar grammar = slr_table_.find_grammar(grammar_id);
  if (grammar == Grammar("D -> T d")) {
    reduce_var_declaration(tokens);
  } else if (grammar == Grammar("D -> T d ( A' ) { D' S' }")) {
    reduce_func_declaration(tokens);
  } else if (grammar == Grammar("A' -> ε")) {
    reduce_func_params(tokens);
  }
  return true;
}

void SymbolTableAnalyzer1::push_symbol_table(const std::shared_ptr<SymbolTable>& symbol_table) {
  stack_symbol_table_.push(symbol_table);
}

std::shared_ptr<SymbolTableAnalyzer::SymbolTable> SymbolTableAnalyzer1::pop_symbol_table() {
  if (stack_symbol_table_.empty()) return nullptr;
  auto top = stack_symbol_table_.top();
  stack_symbol_table_.pop();
  return top;
}

std::string SymbolTableAnalyzer::get_table_name(const SymbolTable& symbol_table) {
  std::string name = symbol_table.name;
  TablePtr outer = symbol_table.outer;
  while (outer) {
    name += ("@" + outer->name);
    outer = outer->outer;
    break; // 这里暂时只找上一层的名称
  }
  return name;
}

