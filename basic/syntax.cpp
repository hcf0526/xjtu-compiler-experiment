//
// Created by WangLele on 25-5-2.
//

#include "syntax.hpp"
#include "utils/strtool.hpp"

void Syntax::SymbolTable::add_entry(const std::shared_ptr<Entry> &entry) {
  // 检查是否重名
  for (const auto& e : entries) {
    if (e->name == entry->name) {
      throw std::runtime_error("Symbol " + entry->name + " is already defined in this scope.");
    }
  }
  entries.push_back(entry);
}

std::shared_ptr<Syntax::Entry> Syntax::SymbolTable::lookup_entry(const std::string &name) {
  for (const auto& entry : entries) {
    if (entry->name == name) {
      return entry;
    }
  }
  return nullptr; // 未找到
}

std::ostream &operator<<(std::ostream &os, const Syntax::Entry &entry) {
  os << "(name: " << entry.name << ", type: " << strtool::to_upper(entry.type)
     << ", offset: " << entry.offset << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os, const Syntax::ArrayEntry &entry) {
  os << "(name: " << entry.name << ", type: " << strtool::to_upper(entry.type)
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
  os << "(name: " << entry.name << ", type: " << strtool::to_upper(entry.type)
     << ", offset: " << entry.offset << ", mytab: "
     << Syntax::get_table_name(*entry.mytab) << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os, const SyntaxZyl::ArrayPttEntry &entry) {
  os << "(name: " << entry.name << ", type: " << strtool::to_upper(entry.type)
     << ", etype: " << strtool::to_upper(entry.etype) << ", base: " << entry.base
     << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os, const SyntaxZyl::FuncPttEntry &entry) {
  os << "(name: " << entry.name << ", type: " << strtool::to_upper(entry.type)
     << ", offset: " << entry.offset << ", rtype: " << strtool::to_upper(entry.rtype)
     << ")";
  return os;
}

std::ostream &operator<<(std::ostream &os, const Syntax::SymbolWithAttribute &symbol) {
  os << "(" << symbol.name << ", " << symbol.value << ")";
  return os;
}

std::ostream & operator<<(std::ostream &os, const Syntax::SymbolTable &symbol_table) {
  os << (Syntax::get_table_name(symbol_table)) << ": {" << std::endl;
  os << "  width: " << symbol_table.width
     << " argc: " << symbol_table.argc
     << " rtype: " << strtool::to_upper(symbol_table.rtype)
     << " level: " << symbol_table.level
     << std::endl;
  // 参数列表
  os << "  arglist: ";
  os << " (";
  for (size_t i = 0; i < symbol_table.arglist.size(); ++i) {
    os << symbol_table.arglist[i];
    if (i + 1 < symbol_table.arglist.size()) {
      os << ", ";
    }
  }
  os << ")";
  os << std::endl;
  os << "  entries: {" << std::endl;
  for (const auto& entry : symbol_table.entries) {
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
  os << "  code: [" << std::endl;
  for (const auto& code : symbol_table.code) {
    os << code;
  }
  os << "  ]" << std::endl;
  os << "}" << std::endl;
  return os;
}

std::ostream & operator<<(std::ostream &os, const Syntax::Process &p) {
  os << "(" << p.state << ", " << p.token << ", " << p.action << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Syntax& analyzer) {
  for (const auto& process : analyzer.processes_) {
    os << process << "\n";
  }
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
    std::cerr << "[Syntax] symbol_table_to_txt(): 无法打开文件 " << filename << std::endl;
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
  std::stack<SymbolPtr> stack_token;

  // 初始状态入栈
  state_stack.push(slr_table_.start_state());

  size_t pos = 0;

  while (true) {
    // 跳过换行符
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

      std::vector<SymbolPtr> reduced_tokens;

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
      SymbolPtr lhs_symbol = reduce(action.target, reduced_tokens);

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

std::unordered_map<std::string, Syntax::TablePtr> Syntax::symbol_table() const {
  return map_symbol_table_;
}

// 构造函数

SyntaxZyl::SyntaxZyl(const SLRTable& slr_table) : Syntax(slr_table) {
  // 添加默认的系统符号表
  TablePtr system_table = std::make_shared<SymbolTable>("system_table");
  system_table->rtype = "VOID";
  stack_symbol_table_.push(system_table);
  map_symbol_table_["system_table"] = system_table;
  // 添加文法到属性方程的映射
  // 文法 1
  add_equation(Grammar("P -> D' S'"), &SyntaxZyl::reduce_program);
  // 文法 4
  add_equation(Grammar("D -> T d"), &SyntaxZyl::reduce_declaration_var);
  // 文法 5
  add_equation(Grammar("D -> T d [ i ]"), &SyntaxZyl::reduce_declaration_array);
  // 文法 6
  add_equation(Grammar("D -> T d ( A' ) { D' S' }"), &SyntaxZyl::reduce_declaration_func);
  // 文法 7
  add_equation(Grammar("T -> int"), &SyntaxZyl::reduce_type);
  // 文法 8
  add_equation(Grammar("T -> void"), &SyntaxZyl::reduce_type);
  // 文法 9
  add_equation(Grammar("A' -> ε"), &SyntaxZyl::reduce_params);
  // 文法 10
  add_equation(Grammar("A' -> A' A ;"), &SyntaxZyl::reduce_params_list);
  // 文法 11
  add_equation(Grammar("A -> T d"), &SyntaxZyl::reduce_param_var);
  // 文法 12
  add_equation(Grammar("A -> T d [ ]"), &SyntaxZyl::reduce_param_array);
  // 文法 13
  add_equation(Grammar("A -> T d ( )"), &SyntaxZyl::reduce_param_func);
  // 文法 14
  add_equation(Grammar("S' -> S"), &SyntaxZyl::reduce_sentences);
  // 文法 15
  add_equation(Grammar("S' -> S' ; S"), &SyntaxZyl::reduce_sentences_list);
  // 文法 16
  add_equation(Grammar("S -> d = E"), &SyntaxZyl::reduce_sentence_assign);
  // 文法 17
  add_equation(Grammar("S -> if ( B ) S"), &SyntaxZyl::reduce_sentence_if);
  // 文法 18
  add_equation(Grammar("S -> if ( B ) S else S"), &SyntaxZyl::reduce_sentence_if_else);
  // 文法 19
  add_equation(Grammar("S -> while ( B ) S"), &SyntaxZyl::reduce_sentence_while);
  // 文法 20
  add_equation(Grammar("S -> return E"), &SyntaxZyl::reduce_sentence_return);
  // 文法 21
  add_equation(Grammar("S -> { S' }"), &SyntaxZyl::reduce_sentence_block);
  // 文法 22
  add_equation(Grammar("S -> d ( R' )"), &SyntaxZyl::reduce_sentence_call);
  // 文法 23
  add_equation(Grammar("B -> B ∧ B"), &SyntaxZyl::reduce_bool_and);
  // 文法 24
  add_equation(Grammar("B -> B ∨ B"), &SyntaxZyl::reduce_bool_or);
  // 文法 25
  add_equation(Grammar("B -> E r E"), &SyntaxZyl::reduce_bool_relation);
  // 文法 26
  add_equation(Grammar("B -> E"), &SyntaxZyl::reduce_bool_expr);
  // 文法 27
  add_equation(Grammar("E -> d = E"), &SyntaxZyl::reduce_expr_assgin);
  // 文法 28
  add_equation(Grammar("E -> i"), &SyntaxZyl::reduce_expr_num);
  // 文法 29
  add_equation(Grammar("E -> d"), &SyntaxZyl::reduce_expr_var);
  // 文法 30
  add_equation(Grammar("E -> d ( R' )"), &SyntaxZyl::reduce_expr_call);
  // 文法 31
  add_equation(Grammar("E -> E + E"), &SyntaxZyl::reduce_expr_add);
  // 文法 32
  add_equation(Grammar("E -> E * E"), &SyntaxZyl::reduce_expr_mul);
  // 文法 33
  add_equation(Grammar("E -> ( E )"), &SyntaxZyl::reduce_expr_bracket);
  // 文法 34
  add_equation(Grammar("R' -> ε"), &SyntaxZyl::reduce_call_params);
  // 文法 35
  add_equation(Grammar("R' -> R' R ,"), &SyntaxZyl::reduce_call_params_list);
  // 文法 36
  add_equation(Grammar("R -> E"), &SyntaxZyl::reduce_call_param_expr);
  // 文法 37
  add_equation(Grammar("R -> d [ ]"), &SyntaxZyl::reduce_call_param_array);
  // 文法 38
  add_equation(Grammar("R -> d ( )"), &SyntaxZyl::reduce_call_param_func);
}

// 根据不同的属性方程, 由程序员实现函数的定义, 父类提供接口.
// 否则, 对属性方程过于抽象, 不容易实现.

bool SyntaxZyl::shift(int state_id, const Lexical::Token &token) {
  return true;
}

Syntax::SymbolPtr SyntaxZyl::reduce_program(const std::vector<SymbolPtr> &symbols) {
  // 文法 1
  // P -> D' S'
  //      0  1
  SymbolSCPtr symbol_sc = std::dynamic_pointer_cast<SymbolSC>(symbols[1]);
  TablePtr symbol_table = stack_symbol_table_.top();
  symbol_table->code = symbol_sc->code;
  SymbolP symbol_p = SymbolP("P", "P", {symbol_sc->code});
  return std::make_shared<SymbolP>(symbol_p);
}

Syntax::SymbolPtr SyntaxZyl::reduce_declaration_var(const std::vector<SymbolPtr> &symbols) {
  // 文法 4
  // D -> T d
  //      0 1
  TablePtr symbol_table = stack_symbol_table_.top();
  EntryPtr entry = std::make_shared<Entry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);

  entry->name = symbols[1]->value;
  entry->type = symbol_t->type;

  symbol_table->width += size_of(symbol_t->type);
  symbol_table->add_entry(entry);

  entry->offset = symbol_table->width;

  SymbolD symbol_d = SymbolD("D", "D", {symbols[1]->value});
  return std::make_shared<SymbolD>(symbol_d);
}

Syntax::SymbolPtr SyntaxZyl::reduce_declaration_array(const std::vector<SymbolPtr> &symbols) {
  // 文法 5
  // D -> T d [ i ]
  //      0 1 2 3 4
  TablePtr symbol_table = stack_symbol_table_.top();
  ArrayEntryPtr entry = std::make_shared<ArrayEntry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);
  int array_length = std::stoi(symbols[3]->value);

  entry->name = symbols[1]->value;
  entry->type = "array";
  entry->dims = 1;
  entry->dim.push_back(array_length);
  entry->etype = symbol_t->type;

  symbol_table->width += array_length * size_of(symbol_t->type);
  symbol_table->add_entry(entry);

  entry->base = symbol_table->width;

  SymbolD symbol_d = SymbolD("D", "D", {symbols[1]->value});
  return std::make_shared<SymbolD>(symbol_d);
}

Syntax::SymbolPtr SyntaxZyl::reduce_declaration_func(const std::vector<SymbolPtr> &symbols) {
  // 文法 6
  // D -> T d ( A' ) { D' S' }
  //      0 1 2 3  4 5 6  7  8
  TablePtr symbol_table = pop_symbol_table();
  TablePtr outer = stack_symbol_table_.top();

  // 创建新的符号表
  SymbolSCPtr symbol_sc = std::dynamic_pointer_cast<SymbolSC>(symbols[7]);
  symbol_table->name = symbols[1]->value;
  symbol_table->outer = outer;
  symbol_table->level = outer->level + 1;
  symbol_table->code = symbol_sc->code;
  map_symbol_table_[get_table_name(*symbol_table)] = symbol_table;

  // 创建函数登记项
  FuncEntryPtr entry = std::make_shared<FuncEntry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);

  entry->name = symbols[1]->value;
  entry->type = "func";
  entry->mytab = symbol_table;

  outer->width += size_of("func");
  outer->add_entry(entry);

  entry->offset = outer->width;

  symbol_table->rtype = symbol_t->type;

  SymbolD symbol_d = SymbolD("D", "D", {symbols[1]->value});
  return std::make_shared<SymbolD>(symbol_d);
}

Syntax::SymbolPtr SyntaxZyl::reduce_type(const std::vector<SymbolPtr> &symbols) {
  // 文法 7, 8
  // T -> int
  // T -> void
  SymbolT symbol_t = SymbolT("T", "T", symbols[0]->value);
  return std::make_shared<SymbolT>(symbol_t);
}

Syntax::SymbolPtr SyntaxZyl::reduce_params(const std::vector<SymbolPtr> &symbols) {
  // 文法 9
  // A' -> ε
  //       0
  TablePtr symbol_table = std::make_shared<SymbolTable>();
  stack_symbol_table_.push(symbol_table);

  SymbolAC symbol_a = SymbolAC("A'", "A'", {});
  return std::make_shared<SymbolAC>(symbol_a);
}

Syntax::SymbolPtr SyntaxZyl::reduce_params_list(const std::vector<SymbolPtr> &symbols) {
  // 文法 10
  // A' -> A' A ;
  //       0  1 2
  SymbolACPtr symbol_ac = std::dynamic_pointer_cast<SymbolAC>(symbols[0]);
  SymbolAPtr symbol_a = std::dynamic_pointer_cast<SymbolA>(symbols[1]);
  // 将两个place连起来
  std::vector<std::string> new_place = symbol_a->place;
  new_place.insert(new_place.end(), symbol_ac->place.begin(), symbol_ac->place.end());
  SymbolAC symbol_ac_new = SymbolAC("A'", "A'", new_place);
  return std::make_shared<SymbolAC>(symbol_ac_new);
}

Syntax::SymbolPtr SyntaxZyl::reduce_param_var(const std::vector<SymbolPtr> &symbols) {
  // 文法 11
  // A -> T d
  //      0 1
  TablePtr symbol_table = stack_symbol_table_.top();
  EntryPtr entry = std::make_shared<Entry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);

  entry->name = symbols[1]->value;
  entry->type = symbol_t->type;

  symbol_table->argc++;
  symbol_table->width += size_of(symbol_t->type);
  symbol_table->arglist.push_back(symbols[1]->value);
  symbol_table->add_entry(entry);

  entry->offset = symbol_table->width;

  SymbolA symbol_a = SymbolA("A", "A", {symbols[1]->value});
  return std::make_shared<SymbolA>(symbol_a);
}

Syntax::SymbolPtr SyntaxZyl::reduce_param_array(const std::vector<SymbolPtr> &symbols) {
  // 文法 12
  // A -> T d [ ]
  //      0 1 2 3
  TablePtr symbol_table = stack_symbol_table_.top();
  ArrayPttEntryPtr entry = std::make_shared<ArrayPttEntry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);

  entry->name = symbols[1]->value;
  entry->type = "arrptt";
  entry->etype = symbol_t->type;

  symbol_table->argc++;
  symbol_table->width += size_of(symbol_t->type);
  symbol_table->arglist.push_back(symbols[1]->value);
  symbol_table->add_entry(entry);

  entry->base = symbol_table->width;

  SymbolA symbol_a = SymbolA("A", "A", {symbols[1]->value});
  return std::make_shared<SymbolA>(symbol_a);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_param_func(const std::vector<SymbolPtr> &symbols) {
  // 文法 13
  // A -> T d ( )
  //      0 1 2 3
  TablePtr symbol_table = stack_symbol_table_.top();
  FuncPttEntryPtr entry = std::make_shared<FuncPttEntry>();
  SymbolTPtr symbol_t = std::dynamic_pointer_cast<SymbolT>(symbols[0]);

  entry->name = symbols[1]->value;
  entry->type = "funptt";
  entry->rtype = symbol_t->type;

  symbol_table->argc++;
  symbol_table->width += size_of(symbol_t->type);
  symbol_table->arglist.push_back(symbols[1]->value);
  symbol_table->add_entry(entry);

  entry->offset = symbol_table->width;

  SymbolA symbol_a = SymbolA("A", "A", {symbols[1]->value});
  return std::make_shared<SymbolA>(symbol_a);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_sentences(const std::vector<SymbolPtr> &symbols) {
  // 文法 14
  // S' -> S
  //       0
  SymbolSPtr symbol_s = std::dynamic_pointer_cast<SymbolS>(symbols[0]);
  SymbolSC symbol_sc = SymbolSC("S'", "S'", {symbol_s->code});
  return std::make_shared<SymbolSC>(symbol_sc);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_sentences_list(const std::vector<SymbolPtr> &symbols) {
  // 文法 15
  // S' -> S' ; S
  //       0  1 2
  SymbolSCPtr symbol_sc = std::dynamic_pointer_cast<SymbolSC>(symbols[0]);
  SymbolSPtr symbol_s = std::dynamic_pointer_cast<SymbolS>(symbols[2]);

  std::vector<std::string> new_code = symbol_sc->code;
  new_code.push_back(symbol_s->code);
  SymbolSC symbol_sc_new = SymbolSC("S'", "S'", new_code);
  return std::make_shared<SymbolSC>(symbol_sc_new);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_sentence_assign(const std::vector<SymbolPtr> &symbols) {
  // 文法 16
  // S -> d = E
  //      0 1 2
  std::string var_name = symbols[0]->value;
  SymbolEPtr symbol_e = std::dynamic_pointer_cast<SymbolE>(symbols[2]);

  std::string code = symbol_e->code;
  code += var_name + " = " + symbol_e->place + ";\n";
  SymbolS symbol_s = SymbolS("S", "S", code);
  return std::make_shared<SymbolS>(symbol_s);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_sentence_if(const std::vector<SymbolPtr> &symbols) {
  // 文法 17
  // S -> if ( B ) S
  //      0  1 2 3 4
  SymbolBPtr symbol_b = std::dynamic_pointer_cast<SymbolB>(symbols[2]);
  SymbolSPtr symbol_s = std::dynamic_pointer_cast<SymbolS>(symbols[4]);

  std::string code = symbol_b->code;
  code += gen_code("LABEL", symbol_b->tc);
  code += symbol_s->code + "\n";
  code += gen_code("LABEL", symbol_b->fc);

  SymbolS symbol_s_new = SymbolS("S", "S", code);
  return std::make_shared<SymbolS>(symbol_s_new);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_sentence_if_else(const std::vector<SymbolPtr> &symbols) {
  // 文法 18
  // S -> if ( B ) S else S
  //      0  1 2 3 4 5    6
  std::string label = new_label();
  SymbolBPtr symbol_b = std::dynamic_pointer_cast<SymbolB>(symbols[2]);
  SymbolSPtr symbol_s1 = std::dynamic_pointer_cast<SymbolS>(symbols[4]);
  SymbolSPtr symbol_s2 = std::dynamic_pointer_cast<SymbolS>(symbols[6]);

  std::string code = symbol_b->code;
  code += gen_code("LABEL", symbol_b->tc);
  code += symbol_s1->code;
  code += gen_code("GOTO", label);
  code += gen_code("LABEL", symbol_b->fc);
  code += symbol_s2->code;
  code += gen_code("LABEL", label);

  SymbolS symbol_s_new = SymbolS("S", "S", code);
  return std::make_shared<SymbolS>(symbol_s_new);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_sentence_while(const std::vector<SymbolPtr> &symbols) {
  // 文法 19
  // S -> while ( B ) S
  //      0     1 2 3 4
  std::string label = new_label();
  SymbolBPtr symbol_b = std::dynamic_pointer_cast<SymbolB>(symbols[2]);
  SymbolSPtr symbol_s = std::dynamic_pointer_cast<SymbolS>(symbols[4]);

  std::string code = gen_code("LABEL", {label});
  code += symbol_b->code;
  code += gen_code("LABEL", symbol_b->tc);
  code += symbol_s->code;
  code += gen_code("GOTO", label);
  code += gen_code("LABEL", symbol_b->fc);

  SymbolS symbol_s_new = SymbolS("S", "S", code);
  return std::make_shared<SymbolS>(symbol_s_new);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_sentence_return(const std::vector<SymbolPtr> &symbols) {
  // 文法 20
  // S -> return E
  //      0      1
  SymbolEPtr symbol_e = std::dynamic_pointer_cast<SymbolE>(symbols[1]);

  std::string code = symbol_e->code;
  code += gen_code("RETURN", symbol_e->place);

  SymbolS symbol_s = SymbolS("S", "S", code);
  return std::make_shared<SymbolS>(symbol_s);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_sentence_block(const std::vector<SymbolPtr> &symbols) {
  // 文法 21
  // S -> { S' }
  //      0 1  2
  SymbolSCPtr symbol_sc = std::dynamic_pointer_cast<SymbolSC>(symbols[1]);

  SymbolS symbol_s = SymbolS("S", "S", merge_code(symbol_sc->code));
  return std::make_shared<SymbolS>(symbol_s);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_sentence_call(const std::vector<SymbolPtr> &symbols) {
  // 文法 22
  // S -> d ( R' )
  //      0 1 2  3
  std::string var_name = symbols[0]->value;
  SymbolRCPtr symbol_rc = std::dynamic_pointer_cast<SymbolRC>(symbols[2]);

  std::string new_var_name = new_var();
  std::string code = new_params(symbol_rc->place);
  code += new_var_name + " = " + "CALL " + var_name + ", " + std::to_string(symbol_rc->place.size()) + ";\n";

  std::string symbol_s_code = merge_code(symbol_rc->code);
  symbol_s_code += code;

  SymbolS symbol_s = SymbolS("S", "S", symbol_s_code);
  return std::make_shared<SymbolS>(symbol_s);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_bool_and(const std::vector<SymbolPtr> &symbols) {
  // 文法 23
  // B -> B ∧ B
  //      0 1 2
  SymbolBPtr symbol_b1 = std::dynamic_pointer_cast<SymbolB>(symbols[0]);
  SymbolBPtr symbol_b2 = std::dynamic_pointer_cast<SymbolB>(symbols[2]);

  std::vector<std::string> tc = symbol_b2->tc;
  std::vector<std::string> fc = merge_list(symbol_b1->fc, symbol_b2->fc);

  std::string code = symbol_b1->code;
  code += gen_code("LABEL", symbol_b1->tc);
  code += symbol_b2->code;

  SymbolB symbol_b = SymbolB("B", "B", tc, fc, code);
  return std::make_shared<SymbolB>(symbol_b);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_bool_or(const std::vector<SymbolPtr> &symbols) {
  // 文法 24
  // B -> B ∨ B
  //      0 1 2
  SymbolBPtr symbol_b1 = std::dynamic_pointer_cast<SymbolB>(symbols[0]);
  SymbolBPtr symbol_b2 = std::dynamic_pointer_cast<SymbolB>(symbols[2]);

  std::vector<std::string> tc = merge_list(symbol_b1->tc, symbol_b2->tc);
  std::vector<std::string> fc = symbol_b2->fc;

  std::string code = symbol_b1->code;
  code += gen_code("LABEL", symbol_b1->fc);
  code += symbol_b2->code;

  SymbolB symbol_b = SymbolB("B", "B", tc, fc, code);
  return std::make_shared<SymbolB>(symbol_b);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_bool_relation(const std::vector<SymbolPtr> &symbols) {
  // 文法 25
  // B -> E r E
  //      0 1 2
  std::string label1 = new_label();
  std::string label2 = new_label();
  SymbolEPtr symbol_e1 = std::dynamic_pointer_cast<SymbolE>(symbols[0]);
  SymbolEPtr symbol_e2 = std::dynamic_pointer_cast<SymbolE>(symbols[2]);

  std::vector<std::string> tc = { label1 };
  std::vector<std::string> fc = { label2 };
  std::string code = symbol_e1->code + symbol_e2->code;
  code += "IF " + symbol_e1->place + " " + symbols[1]->value + " " + symbol_e2->place + " ";
  code += "THEN " + label1 + " ELSE " + label2 + ";\n";

  SymbolB symbol_b = SymbolB("B", "B", tc, fc, code);
  return std::make_shared<SymbolB>(symbol_b);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_bool_expr(const std::vector<SymbolPtr> &symbols) {
  // 文法 26
  // B -> E
  //      0
  std::cout << "B -> E" << std::endl;
  std::string label1 = new_label();
  std::string label2 = new_label();
  SymbolEPtr symbol_e = std::dynamic_pointer_cast<SymbolE>(symbols[0]);
  std::cout << "B code: " << symbol_e->code << std::endl;

  std::vector<std::string> tc = { label1 };
  std::vector<std::string> fc = { label2 };
  std::string code = symbol_e->code;
  code += "IF " + symbol_e->place + " != 0 ";
  code += "THEN " + label1 + " ELSE " + label2 + ";\n";

  std::cout << "B tc: " << tc.size() << std::endl;
  std::cout << "B fc: " << fc.size() << std::endl;

  SymbolB symbol_b = SymbolB("B", "B", tc, fc, code);
  return std::make_shared<SymbolB>(symbol_b);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_expr_assgin(const std::vector<SymbolPtr> &symbols) {
  // 文法 27
  // E -> d = E
  //      0 1 2
  std::string var_name = symbols[0]->value;
  SymbolEPtr symbol_e = std::dynamic_pointer_cast<SymbolE>(symbols[2]);

  std::string code = symbol_e->code;
  code += var_name + " = " + symbol_e->place + ";\n";

  SymbolE symbol_e_new = SymbolE("E", "E", var_name, code);
  return std::make_shared<SymbolE>(symbol_e_new);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_expr_num(const std::vector<SymbolPtr> &symbols) {
  // 文法 28
  // E -> i
  //      0
  std::string var_name = new_var();

  std::string code = var_name + " = " + symbols[0]->value + ";\n";

  SymbolE symbol_e = SymbolE("E", "E", var_name, code);
  return std::make_shared<SymbolE>(symbol_e);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_expr_var(const std::vector<SymbolPtr> &symbols) {
  // 文法 29
  // E -> d
  //      0
  std::string var_name = symbols[0]->value;
  TablePtr symbol_table = stack_symbol_table_.top();

  if (!symbol_table->lookup_entry(var_name)) {
    std::cerr << "[Syntax] 变量 " << var_name << " 未定义." << std::endl;
    throw std::runtime_error("未定义变量" + var_name);
    return nullptr;
  }

  SymbolE symbol_e = SymbolE("E", "E", var_name, "");
  return std::make_shared<SymbolE>(symbol_e);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_expr_call(const std::vector<SymbolPtr> &symbols) {
  // 文法 30
  // E -> d ( R' )
  //      0 1 2  3
  std::string var_name = symbols[0]->value;
  SymbolRCPtr symbol_rc = std::dynamic_pointer_cast<SymbolRC>(symbols[2]);

  std::string new_var_name = new_var();
  std::string code = new_params(symbol_rc->place);
  code += new_var_name + " = " + "CALL " + var_name + ", " + std::to_string(symbol_rc->place.size()) + ";\n";

  std::string symbol_e_code = merge_code(symbol_rc->code);
  symbol_e_code += code;

  SymbolE symbol_e = SymbolE("E", "E", new_var_name, symbol_e_code);
  return std::make_shared<SymbolE>(symbol_e);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_expr_add(const std::vector<SymbolPtr> &symbols) {
  // 文法 31
  // E -> E + E
  //      0 1 2
  std::string new_var_name = new_var();
  SymbolEPtr symbol_e1 = std::dynamic_pointer_cast<SymbolE>(symbols[0]);
  SymbolEPtr symbol_e2 = std::dynamic_pointer_cast<SymbolE>(symbols[2]);

  std::string code = symbol_e1->code + symbol_e2->code;
  code += new_var_name + " = " + symbol_e1->place + " + " + symbol_e2->place + ";\n";

  SymbolE symbol_e = SymbolE("E", "E", new_var_name, code);
  return std::make_shared<SymbolE>(symbol_e);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_expr_mul(const std::vector<SymbolPtr> &symbols) {
  // 文法 32
  // E -> E * E
  //      0 1 2
  std::string new_var_name = new_var();
  SymbolEPtr symbol_e1 = std::dynamic_pointer_cast<SymbolE>(symbols[0]);
  SymbolEPtr symbol_e2 = std::dynamic_pointer_cast<SymbolE>(symbols[2]);

  std::string code = symbol_e1->code + symbol_e2->code;
  code += new_var_name + " = " + symbol_e1->place + " * " + symbol_e2->place + ";\n";

  SymbolE symbol_e = SymbolE("E", "E", new_var_name, code);
  return std::make_shared<SymbolE>(symbol_e);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_expr_bracket(const std::vector<SymbolPtr> &symbols) {
  // 文法 33
  // E -> ( E )
  //      0 1 2
  SymbolEPtr symbol_e = std::dynamic_pointer_cast<SymbolE>(symbols[1]);

  SymbolE symbol_e_new = SymbolE("E", "E", symbol_e->place, symbol_e->code);
  return std::make_shared<SymbolE>(symbol_e_new);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_call_params(const std::vector<SymbolPtr> &symbols) {
  // 文法 34
  // R' -> ε
  //       0
  SymbolRC symbol_rc = SymbolRC("R'", "R'", {}, {});
  return std::make_shared<SymbolRC>(symbol_rc);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_call_params_list(const std::vector<SymbolPtr> &symbols) {
  // 文法 35
  // R' -> R' R ,
  //       0  1 2
  SymbolRCPtr symbol_rc = std::dynamic_pointer_cast<SymbolRC>(symbols[0]);
  SymbolRPtr symbol_r = std::dynamic_pointer_cast<SymbolR>(symbols[1]);

  std::vector<std::string> place = symbol_rc->place;
  place.push_back(symbol_r->place);

  std::vector<std::string> code = symbol_rc->code;
  code.push_back(symbol_r->code);

  SymbolRC symbol_rc_new = SymbolRC("R'", "R'", place, code);
  return std::make_shared<SymbolRC>(symbol_rc_new);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_call_param_expr(const std::vector<SymbolPtr> &symbols) {
  // 文法 36
  // R -> E
  //      0
  SymbolEPtr symbol_e = std::dynamic_pointer_cast<SymbolE>(symbols[0]);
  SymbolR symbol_r = SymbolR("R", "R", symbol_e->place, symbol_e->code);
  return std::make_shared<SymbolR>(symbol_r);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_call_param_array(const std::vector<SymbolPtr> &symbols) {
  // 文法 37
  // R -> d [ ]
  //      0 1 2
  std::string var_name = symbols[0]->value;
  SymbolR symbol_r = SymbolR("R", "R", var_name, "");
  return std::make_shared<SymbolR>(symbol_r);
}

SyntaxZyl::SymbolPtr SyntaxZyl::reduce_call_param_func(const std::vector<SymbolPtr> &symbols) {
  // 文法 38
  // R -> d ( )
  //      0 1 2
  std::string var_name = symbols[0]->value;
  SymbolR symbol_r = SymbolR("R", "R", var_name, "");
  return std::make_shared<SymbolR>(symbol_r);
}

std::string SyntaxZyl::new_var() {
  return "t" + std::to_string(variable_count_++);
}

std::string SyntaxZyl::new_label() {
  return "l" + std::to_string(label_count_++);
}

std::string SyntaxZyl::gen_code(const std::string &instruction, const std::string &label) {
  return instruction + " " + label + ";\n";
}

std::string SyntaxZyl::gen_code(const std::string& instruction, const std::vector<std::string> &labels) {\
  std::ostringstream oss;
  for (const auto &label : labels) {
    oss << instruction << " " << label << ";\n";
  }
  return oss.str();
}

std::string SyntaxZyl::new_labels(const std::vector<std::string> &labels) {
  std::ostringstream oss;
  for (const auto &label : labels) {
    oss << "LABEL " << label << "; ";
  }
  return oss.str();
}

std::string SyntaxZyl::new_params(const std::vector<std::string> &parlist) {
  std::ostringstream oss;
  for (auto it = parlist.rbegin(); it != parlist.rend(); ++it) {
    oss << "PAR " << *it << ";\n";
  }
  return oss.str();
}

std::vector<std::string> SyntaxZyl::merge_list(const std::vector<std::string> &list1,
  const std::vector<std::string> &list2) {
  std::vector<std::string> merged_list = list1;
  merged_list.insert(merged_list.end(), list2.begin(), list2.end());
  return merged_list;
}

std::string SyntaxZyl::merge_code(const std::vector<std::string> &code_list) {
  std::ostringstream oss;
  for (const auto &code : code_list) {
    oss << code;
  }
  return oss.str();
}

void SyntaxZyl::add_equation(const Grammar &grammar, AttrEqnPtr handler) {
  reduce_map_.emplace(grammar, handler);
}

Syntax::SymbolPtr SyntaxZyl::reduce(int grammar_id, const std::vector<SymbolPtr> &symbols) {
  Grammar grammar = slr_table_.find_grammar(grammar_id);

  auto it = reduce_map_.find(grammar);
  if (it != reduce_map_.end()) {
    AttrEqnPtr handler = it->second;
    return (this->*handler)(symbols);
  }

  return std::make_shared<SymbolWithAttribute>(grammar.lhs(), grammar.lhs());
}

std::shared_ptr<Syntax::SymbolTable> SyntaxZyl::pop_symbol_table() {
  if (stack_symbol_table_.empty()) return nullptr;
  auto top = stack_symbol_table_.top();
  stack_symbol_table_.pop();
  return top;
}

int SyntaxZyl::size_of(const std::string &type) {
  std::string t = strtool::to_lower(type);
  if (t == "int") return 4;
  if (t == "void") return 0;
  if (t == "array" || t == "arrptt" || t == "arrayptt") return 4;
  if (t == "func" || t == "funptt" || t == "funcptt") return 8;
  return 0;
}

std::string Syntax::get_table_name(const SymbolTable& symbol_table) {
  std::string name = symbol_table.name;
  TablePtr outer = symbol_table.outer;
  while (outer) {
    name += ("@" + outer->name);
    outer = outer->outer;
    break; // 这里暂时只找上一层的名称
  }
  return name;
}

