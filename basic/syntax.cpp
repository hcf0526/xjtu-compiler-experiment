//
// Created by WangLele on 25-5-2.
//

#include "syntax.hpp"

void Syntax::SymbolTable::add_entry(const std::shared_ptr<Entry> &entry) {
  // 检查是否重名
  for (const auto& e : entries) {
    if (e->name == entry->name) {
      throw std::runtime_error("Symbol '" + entry->name + "' is already defined in this scope.");
    }
  }
  entries.push_back(entry);
}

std::ostream &operator<<(std::ostream &os, const Syntax::Entry &entry) {
  os << "(name: " << entry.name << ", type: " << entry.type
     << ", offset: " << entry.offset << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os, const Syntax::ArrayEntry &entry) {
  os << "(name: " << entry.name << ", type: " << entry.type
     << ", etype: " << entry.etype << ", base: " << entry.base
     << ", dims: " << entry.dims << ", dim: [";
  for (const auto &d : entry.dim) {
    if (d == entry.dim.back()) {
      os << d;
    } else {
      os << d << ", ";
    }
  }
  os << "])";
  return os;
}

std::ostream &operator<<(std::ostream &os, const Syntax::FuncEntry &entry) {
  os << "(name: " << entry.name << ", type: " << entry.type
     << ", offset: " << entry.offset << ", rtype: " << entry.rtype
     << ", mytab: " << Syntax::get_name(*entry.mytab) << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os, const SyntaxZyl::ArrayPttEntry &entry) {
  os << "(name: " << entry.name << ", type: " << entry.type
     << ", etype: " << entry.etype << ", base: " << entry.base
     << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os, const SyntaxZyl::FuncPttEntry &entry) {
  os << "(name: " << entry.name << ", type: " << entry.type
     << ", offset: " << entry.offset << ", rtype: " << entry.rtype
     << ")";
  return os;
}

std::ostream & operator<<(std::ostream &os, const Syntax::SymbolTable &st) {
  os << (Syntax::get_name(st)) << ": {" << std::endl;
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
    if (entry->type == "array") {
      os << "    " << *std::dynamic_pointer_cast<Syntax::ArrayEntry>(entry) << std::endl;
    } else if (entry->type == "func") {
      os << "    " << *std::dynamic_pointer_cast<Syntax::FuncEntry>(entry) << std::endl;
    } else if (entry->type == "arrayptt") {
      os << "    " << *std::dynamic_pointer_cast<SyntaxZyl::ArrayPttEntry>(entry) << std::endl;
    } else if (entry->type == "funcptt") {
      os << "    " << *std::dynamic_pointer_cast<SyntaxZyl::FuncPttEntry>(entry) << std::endl;
    } else {
      os << "    " << *entry << std::endl;
    }
  }
  os << "  }" << std::endl;
  os << "}" << std::endl;
  return os;
}

std::ostream & operator<<(std::ostream &os, const Syntax::Process &p) {
  os << "(" << p.state << ", " << p.token << ", " << p.action << ")";
  return os;
}

Syntax::Syntax(const SLRTable& slr_table)
  : slr_table_(slr_table) {

}

bool Syntax::parse(const std::vector<Lexical::Token>& tokens) {
  return analyze_tokens(tokens);
}

bool Syntax::parse_file(const std::string& filename, Lexical lexical) {
  const std::vector<Lexical::Token> tokens = lexical.analyze(filename);
  return analyze_tokens(tokens);
}

void Syntax::processes_to_txt(const std::string &filename) const {
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "[错误] processes_to_txt(): 无法打开文件 " << filename << std::endl;
    return;
  }
  file << *this;
}

void Syntax::symbol_table_to_txt(const std::string &filename) const {
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "[错误] symbol_table_to_txt(): 无法打开文件 " << filename << std::endl;
    return;
  }
  for (const auto& [name, table] : map_symbol_table_) {
    file << *table << std::endl;
  }
}

bool Syntax::analyze_tokens(const std::vector<Lexical::Token>& tokens) {
  processes_.clear();

  // 添加输入结束符 #
  std::vector<Lexical::Token> tokens_ = tokens;
  tokens_.push_back({ "#", "#" });

  // 状态栈和符号栈
  std::stack<int> state_stack;
  std::stack<SymAttrPtr> stack_token;

  // 初始状态入栈
  state_stack.push(slr_table_.start_state());

  size_t pos = 0;

  while (true) {
    // 跳过换行符（可选）
    while (pos < tokens_.size() && tokens_[pos].type == "NEWLINE") {
      ++pos;
    }

    // 获取当前状态与当前输入符号
    int current_state = state_stack.top();
    std::string symbol = tokens_[pos].type;

    // 查表获取 ACTION 集合
    const auto* action_set = slr_table_.get_action(current_state, symbol);

    if (!action_set || action_set->empty()) {
      std::cerr << "[错误] 状态 " << current_state << " 符号 '" << symbol << "' 无有效动作。" << std::endl;
      return false;
    }

    if (action_set->size() > 1) {
      std::cerr << "[冲突] 状态 " << current_state << " 符号 '" << symbol << "' 有多个动作，冲突如下：" << std::endl;
      for (const auto& act : *action_set) {
        std::cerr << "  -> " << act << std::endl;
      }
      return false;
    }

    const auto& action = *action_set->begin();
    processes_.push_back({ current_state, tokens_[pos], action });

    if (action.type == SLRTable::SHIFT) {
      // 执行移进动作：状态入栈，符号入栈，向前移动输入
      state_stack.push(action.target);
      stack_token.push(std::make_shared<SymbolWithAttribute>(tokens_[pos]));

      if (!shift(action.target, tokens_[pos])) {
        std::cerr << "[错误] 子类 shift(" << action.target << ") 执行失败。" << std::endl;
        return false;
      }

      ++pos;
    }
    else if (action.type == SLRTable::REDUCE) {
      // 执行归约动作
      const Grammar& g = slr_table_.find_grammar(action.target);
      const auto& rhs = g.rhs();

      bool is_epsilon = (rhs.empty() || (rhs.size() == 1 && rhs[0] == "ε"));

      std::vector<SymAttrPtr> reduced_tokens;

      // 弹出归约右部符号
      if (!is_epsilon) {
        for (size_t i = 0; i < rhs.size(); ++i) {
          if (!state_stack.empty()) state_stack.pop();
          if (!stack_token.empty()) {
            reduced_tokens.push_back(stack_token.top());
            stack_token.pop();
          }
        }
        std::reverse(reduced_tokens.begin(), reduced_tokens.end());
      }

      if (state_stack.empty()) {
        std::cerr << "[错误] 归约时状态栈为空。" << std::endl;
        return false;
      }

      // 查 GOTO 表获取新状态
      int top_state = state_stack.top();
      const auto* goto_set = slr_table_.get_goto(top_state, g.lhs());

      if (!goto_set || goto_set->empty()) {
        std::cerr << "[错误] GOTO(" << top_state << ", " << g.lhs() << ") 无目标状态。" << std::endl;
        return false;
      }

      // 新状态入栈
      state_stack.push(*goto_set->begin());

      // 调用 reduce，生成归约后的符号对象
      SymAttrPtr lhs_symbol = reduce(action.target, reduced_tokens);

      // 判断返回是否有效
      if (!lhs_symbol || lhs_symbol->name.empty()) {
        std::cerr << "[错误] 归约 " << action.target << " 错误，返回空符号。" << std::endl;
        return false;
      }

      // 归约后的非终结符入栈
      stack_token.push(lhs_symbol);
    }
    else if (action.type == SLRTable::ACCEPT) {
      // 接受状态
      std::cout << "[信息] 输入分析成功，状态 " << current_state << " 接受。" << std::endl;
      return true;
    }
    else {
      std::cerr << "[错误] 未知动作类型。" << std::endl;
      return false;
    }
  }
}

std::ostream &operator<<(std::ostream &os, const Syntax::SymbolWithAttribute &swa) {
  os << "(" << swa.name << ", " << swa.value << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Syntax& analyzer) {
  for (const auto& process : analyzer.processes_) {
    os << process << "\n";
  }
  return os;
}

SyntaxZyl::SyntaxZyl(const SLRTable& slr_table) : Syntax(slr_table) {
  std::shared_ptr<SymbolTable> system_table = std::make_shared<SymbolTable>("system_table");
  push_symbol_table(system_table);
  map_symbol_table_["system_table"] = system_table;
}

std::unordered_map<std::string, Syntax::TablePtr> SyntaxZyl::symbol_table() const {
  return map_symbol_table_;
}

int SyntaxZyl::size_of(const std::string &type) {
  if (type == "int") return 4;
  if (type == "void") return 0;
  if (type == "array" || type == "arrayptt") return 4;
  if (type == "func" || type == "funcptt") return 8;
  return 0;
}

// 根据不同的属性方程, 由程序员实现函数的定义, 父类提供接口.
// 否则, 对属性方程过于抽象, 不容易实现.

bool SyntaxZyl::shift(int state_id, const Lexical::Token &token) {
  return true;
}

Syntax::SymAttrPtr SyntaxZyl::reduce_var_declaration(const std::vector<SymAttrPtr>& symbols) {
  // 文法 4
  // D -> T d
  //      0 1
  TablePtr symbol_table = stack_symbol_table_.top();
  EntryPtr entry = std::make_shared<Entry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);
  entry->name = symbols[1]->value;
  entry->type = symbol_t->type;
  symbol_table->width += size_of(symbol_t->type);
  entry->offset = symbol_table->width;
  symbol_table->add_entry(entry);
  return std::make_shared<SymbolWithAttribute>("D", "D");
}

Syntax::SymAttrPtr SyntaxZyl::reduce_array_declaration(const std::vector<SymAttrPtr> &symbols) {
  // 文法 5
  // D -> T d [ i ]
  //      0 1 2 3 4
  TablePtr symbol_table = stack_symbol_table_.top();
  ArrayEntryPtr entry = std::make_shared<ArrayEntry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);
  entry->name = symbols[1]->value;
  entry->type = "array";
  entry->dims = 1;
  int array_length = std::stoi(symbols[3]->value);
  entry->dim.push_back(array_length);
  entry->etype = symbol_t->type;
  symbol_table->width += array_length * size_of(symbol_t->type);
  entry->base = symbol_table->width;
  symbol_table->add_entry(entry);
  return std::make_shared<SymbolWithAttribute>("D", "D");
}

Syntax::SymAttrPtr SyntaxZyl::reduce_func_declaration(const std::vector<SymAttrPtr>& symbols) {
  // 文法 6
  // D -> T d ( A' ) { D' S' }
  //      0 1 2 3  4 5 6  7  8
  TablePtr symbol_table = pop_symbol_table();
  TablePtr outer = stack_symbol_table_.top();
  symbol_table->name = symbols[1]->value;
  symbol_table->outer = outer;
  symbol_table->level = outer->level + 1;
  map_symbol_table_[get_name(*symbol_table)] = symbol_table;
  // 创建函数登记项
  FuncEntryPtr entry = std::make_shared<FuncEntry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);
  entry->name = symbols[1]->value;
  entry->type = "func";
  entry->rtype = symbol_t->type;
  symbol_table->rtype = symbol_t->type;
  entry->mytab = symbol_table;
  outer->width += size_of("func");
  entry->offset = outer->width;
  outer->add_entry(entry);
  return std::make_shared<SymbolWithAttribute>("D", "D");
}

SyntaxZyl::SymbolTPtr SyntaxZyl::reduce_type(const std::vector<SymAttrPtr> &symbols) {
  // 文法 7, 8
  // T -> int
  // T -> void
  SymbolT symbol_t = SymbolT("T", "T", symbols[0]->value);
  return std::make_shared<SymbolT>(symbol_t);
}

Syntax::SymAttrPtr SyntaxZyl::reduce_params(const std::vector<SymAttrPtr>& symbols) {
  // 文法 9
  // A' -> ε
  //       0
  TablePtr symbol_table = std::make_shared<SymbolTable>();
  push_symbol_table(symbol_table);
  return std::make_shared<SymbolWithAttribute>("A'", "A'");
}

Syntax::SymAttrPtr SyntaxZyl::reduce_params_var(const std::vector<SymAttrPtr> &symbols) {
  // 文法 11
  // A -> T d
  //      0 1
  TablePtr symbol_table = stack_symbol_table_.top();
  EntryPtr entry = std::make_shared<Entry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);
  entry->name = symbols[1]->value;
  entry->type = symbol_t->type;
  symbol_table->width += size_of(symbol_t->type);
  entry->offset = symbol_table->width;
  symbol_table->argc++;
  symbol_table->arglist.push_back(symbols[1]->value);
  symbol_table->add_entry(entry);
  return std::make_shared<SymbolWithAttribute>("A", "A");
}

Syntax::SymAttrPtr SyntaxZyl::reduce_params_array(const std::vector<SymAttrPtr> &symbols) {
  // 文法 12
  // A -> T d [ ]
  //      0 1 2 3
  TablePtr symbol_table = stack_symbol_table_.top();
  ArrayPttEntryPtr entry = std::make_shared<ArrayPttEntry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);
  entry->name = symbols[1]->value;
  entry->type = "arrayptt";
  symbol_table->width += size_of(symbol_t->type);
  entry->base = symbol_table->width;
  entry->etype = symbol_t->type;
  symbol_table->argc++;
  symbol_table->arglist.push_back(symbols[1]->value);
  symbol_table->add_entry(entry);
  return std::make_shared<SymbolWithAttribute>("A", "A");
}

Syntax::SymAttrPtr SyntaxZyl::reduce_params_func(const std::vector<SymAttrPtr> &symbols) {
  // 文法 13
  // A -> T d ( )
  //      0 1 2 3
  TablePtr symbol_table = stack_symbol_table_.top();
  FuncPttEntryPtr entry = std::make_shared<FuncPttEntry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);
  entry->name = symbols[1]->value;
  entry->type = "funcptt";
  symbol_table->width += size_of(symbol_t->type);
  entry->offset = symbol_table->width;
  entry->rtype = symbol_t->type;
  symbol_table->argc++;
  symbol_table->arglist.push_back(symbols[1]->value);
  symbol_table->add_entry(entry);
  return std::make_shared<SymbolWithAttribute>("A", "A");
}

Syntax::SymAttrPtr SyntaxZyl::reduce(int grammar_id, const std::vector<SymAttrPtr> &symbols) {
  Grammar grammar = slr_table_.find_grammar(grammar_id);
  if (grammar == Grammar("D -> T d")) {
    return reduce_var_declaration(symbols);
  }
  if (grammar == Grammar("D -> T d [ i ]")) {
    return reduce_array_declaration(symbols);
  }
  if (grammar == Grammar("D -> T d ( A' ) { D' S' }")) {
    return reduce_func_declaration(symbols);
  }
  if (grammar == Grammar("A' -> ε")) {
    return reduce_params(symbols);
  }
  if (grammar == Grammar("T -> int") ||
      grammar == Grammar("T -> void")) {
    return reduce_type(symbols);
  }
  if (grammar == Grammar("A -> T d")) {
    return reduce_params_var(symbols);
  }
  if (grammar == Grammar("A -> T d [ ]")) {
    return reduce_params_array(symbols);
  }
  if (grammar == Grammar("A -> T d ( )")) {
    return reduce_params_func(symbols);
  }
  return std::make_shared<SymbolWithAttribute>(grammar.lhs(), grammar.lhs());
}

void SyntaxZyl::push_symbol_table(const std::shared_ptr<SymbolTable>& symbol_table) {
  stack_symbol_table_.push(symbol_table);
}

std::shared_ptr<Syntax::SymbolTable> SyntaxZyl::pop_symbol_table() {
  if (stack_symbol_table_.empty()) return nullptr;
  auto top = stack_symbol_table_.top();
  stack_symbol_table_.pop();
  return top;
}

std::string Syntax::get_name(const SymbolTable& symbol_table) {
  std::string name = symbol_table.name;
  TablePtr outer = symbol_table.outer;
  while (outer) {
    name += ("@" + outer->name);
    outer = outer->outer;
    break; // 这里暂时只找上一层的名称
  }
  return name;
}

