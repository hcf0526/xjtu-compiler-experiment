#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP


#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "slr_table.hpp"
#include "lexical.hpp"

class Syntax {
public:
  struct Table;

  struct Entry {
    std::string name; // 符号名称
    std::string type; // 符号类型
    int offset; // 偏移量
    virtual ~Entry() = default; // 虚析构函数用于支持多态
    friend std::ostream &operator<<(std::ostream &os, const Entry &entry);
  };

  struct ArrayEntry : Entry {
    std::string etype; // 元素类型
    int base;
    int dims;
    std::vector<int> dim;
    friend std::ostream &operator<<(std::ostream &os, const ArrayEntry &entry);
  };

  struct FunEntry : Entry {
    std::shared_ptr<Table> mytab;
    friend std::ostream &operator<<(std::ostream &os, const FunEntry &entry);
  };

  struct ArrPttEntry: Entry {
    std::string etype;
    int base;
    friend std::ostream &operator<<(std::ostream &os, const ArrPttEntry &entry);
  };

  struct FunPttEntry: Entry {
    std::string rtype;
    friend std::ostream &operator<<(std::ostream &os, const FunPttEntry &entry);
  };

  using EntryPtr = std::shared_ptr<struct Syntax::Entry>;
  using ArrEntryPtr = std::shared_ptr<struct Syntax::ArrayEntry>;
  using FunEntryPtr = std::shared_ptr<struct Syntax::FunEntry>;
  using ArrPttEntryPtr = std::shared_ptr<struct ArrPttEntry>;
  using FunPttEntryPtr = std::shared_ptr<struct FunPttEntry>;

  struct Table {
    std::string name; // 符号表名称
    std::shared_ptr<Table> outer = nullptr; // 指向外层符号表
    int width = 0; // 符号表宽度
    int argc = 0; // 参数个数
    std::vector<std::string> arglist; // 参数名称列表
    std::string rtype; // 返回值类型
    int level = 0; // 层级
    std::vector<std::string> code; // 代码
    std::vector<EntryPtr> entries; // 登记项

    explicit Table(std::string name = "") : name(std::move(name)) {} // 构造函数
    void add_entry(const EntryPtr &entry); // 添加登记项
    EntryPtr lookup_entry(const std::string &name); // 查找登记项
    friend std::ostream &operator<<(std::ostream &os, const Table &symbol_table);
  };

  using TablePtr = std::shared_ptr<Table>;

  struct Process {
    int state; // 状态
    Lexical::Token token; // token
    SLRTable::Action action; // 动作
    friend std::ostream &operator<<(std::ostream &os, const Process &p);
  };

  struct Symbol {
    std::string name;
    std::string value;
    Symbol(std::string name, std::string value) : name(std::move(name)), value(std::move(value)) {}
    Symbol(const Lexical::Token& token) : name(token.type), value(token.lexeme) {}
    friend std::ostream &operator<<(std::ostream &os, const Symbol &symbol);
    virtual ~Symbol() = default;
  };

  using SymbolPtr = std::shared_ptr<Symbol>;

  // 构造函数：SLRTable作为语法分析器
  explicit Syntax(const SLRTable &slr_table);

  // 析构函数
  virtual ~Syntax() = default;

  // 解析输入字符串或文件
  bool parse(const std::vector<Lexical::Token> &tokens);
  bool parse_file(const std::string &filename, Lexical lexical);

  std::unordered_map<std::string, TablePtr> symbol_table() const;
  void processes_to_txt(const std::string &filename) const;
  void symbol_table_to_txt(const std::string &filename) const;

protected:
  const SLRTable &slr_table_; // 引用外部的SLR表
  std::vector<Process> processes_; // 语法分析过程记录
  std::unordered_map<std::string, TablePtr> map_symbol_table_; // 符号表映射
  std::stack<std::shared_ptr<Table> > stack_symbol_table_; // 符号表栈

  // 辅助函数：从Token序列进行语义分析
  friend std::ostream &operator<<(std::ostream &os, const FunEntry &entry);
  friend std::ostream &operator<<(std::ostream &os, const Syntax &analyzer);
  friend std::ostream &operator<<(std::ostream &os, const Table &symbol_table);
  static std::string get_table_name(const Table& symbol_table);

  bool analyze_tokens(const std::vector<Lexical::Token> &tokens);

  virtual bool shift(int state_id, const Lexical::Token &token) = 0;

  virtual SymbolPtr reduce(int grammar_id, const std::vector<SymbolPtr> &symbols) = 0;
};

class SyntaxZyl : public Syntax {
public:
  struct SymbolT: Symbol {
    std::string type;
    SymbolT(std::string name, std::string value, std::string type)
        : Symbol(std::move(name), std::move(value)), type(std::move(type)) {}
  };
  using TPtr = std::shared_ptr<struct SymbolT>;

  struct SymbolD: Symbol {
    std::vector<std::string> place;
    SymbolD(std::string name, std::string value, std::vector<std::string> place)
        : Symbol(std::move(name), std::move(value)), place(std::move(place)) {}
  };
  using DPtr = std::shared_ptr<struct SymbolD>;

  struct SymbolA: Symbol {
    std::vector<std::string> place;
    SymbolA(std::string name, std::string value, std::vector<std::string> place)
        : Symbol(std::move(name), std::move(value)), place(std::move(place)) {}
  };
  using APtr = std::shared_ptr<struct SymbolA>;

  struct SymbolAList: Symbol {
    std::vector<std::string> place;
    SymbolAList(std::string name, std::string value, std::vector<std::string> place)
        : Symbol(std::move(name), std::move(value)), place(std::move(place)) {}
  };
  using AListPtr = std::shared_ptr<struct SymbolAList>;

  struct SymbolB: Symbol {
    std::vector<std::string> tc;
    std::vector<std::string> fc;
    std::string code;
    SymbolB(std::string name, std::string value, std::vector<std::string> tc, std::vector<std::string> fc, std::string code)
        : Symbol(std::move(name), std::move(value)), tc(std::move(tc)), fc(std::move(fc)), code(std::move(code)) {}
  };
  using BPtr = std::shared_ptr<struct SymbolB>;

  struct SymbolE: Symbol {
    std::string place;
    std::string code;
    std::string type;
    std::string num;
    SymbolE(std::string name, std::string value, std::string place, std::string code, std::string type, std::string const_number)
        : Symbol(std::move(name), std::move(value)), place(std::move(place)), code(std::move(code)), type(std::move(type)), num(std::move(const_number)) {}
  };
  using EPtr = std::shared_ptr<struct SymbolE>;

  struct SymbolEList: Symbol {
    std::vector<std::string> place;
    std::vector<std::string> code;
    SymbolEList(std::string name, std::string value, std::vector<std::string> place, std::vector<std::string> code)
        : Symbol(std::move(name), std::move(value)), place(std::move(place)), code(std::move(code)) {}
  };
  using EListPtr = std::shared_ptr<struct SymbolEList>;

  struct SymbolP: Symbol {
    std::vector<std::string> code;
    SymbolP(std::string name, std::string value, std::vector<std::string> code)
        : Symbol(std::move(name), std::move(value)), code(std::move(code)) {}
  };
  using PPtr = std::shared_ptr<struct SymbolP>;

  struct SymbolR: Symbol {
    std::string place;
    std::string code;

    SymbolR(std::string name, std::string value, std::string place, std::string code)
        : Symbol(std::move(name), std::move(value)), place(std::move(place)), code(std::move(code)) {}
  };
  using RPtr = std::shared_ptr<struct SymbolR>;

  struct SymbolRList: Symbol {
    std::vector<std::string> place;
    std::vector<std::string> code;
    SymbolRList(std::string name, std::string value, std::vector<std::string> place, std::vector<std::string> code)
        : Symbol(std::move(name), std::move(value)), place(std::move(place)), code(std::move(code)) {}
  };
  using RListPtr = std::shared_ptr<struct SymbolRList>;

  struct SymbolS: Symbol {
    std::string code;
    SymbolS(std::string name, std::string value, std::string code)
        : Symbol(std::move(name), std::move(value)), code(std::move(code)) {}
  };
  using SPtr = std::shared_ptr<struct SymbolS>;

  struct SymbolSList: Symbol {
    std::vector<std::string> code;
    SymbolSList(std::string name, std::string value, std::vector<std::string> code)
        : Symbol(std::move(name), std::move(value)), code(std::move(code)) {}
  };
  using SListPtr = std::shared_ptr<struct SymbolSList>;

  explicit SyntaxZyl(const SLRTable &slr_table);

private:
  unsigned int variable_count_ = 0; // 变量计数器
  unsigned int label_count_ = 0; // 标签计数器
  using AttrEqnFuncPtr = SymbolPtr (SyntaxZyl::*)(const std::vector<SymbolPtr>&);
  std::unordered_map<Grammar, AttrEqnFuncPtr, Grammar::Hash> reduce_map_;

  // 移进/归约
  bool shift(int state_id, const Lexical::Token &token) override;
  SymbolPtr reduce(int grammar_id, const std::vector<SymbolPtr> &symbols) override;
  // 属性方程
  // 文法 1
  // P -> D' S'
  Syntax::SymbolPtr reduce_program(const std::vector<SymbolPtr> &symbols);

  // 文法 4
  // D -> T d
  Syntax::SymbolPtr reduce_declaration_var(const std::vector<SymbolPtr> &symbols);

  // 文法 5
  // D -> T d [ i ]
  Syntax::SymbolPtr reduce_declaration_array(const std::vector<SymbolPtr> &symbols);

  // 文法 6
  // D -> T d ( A' ) { D' S' }
  Syntax::SymbolPtr reduce_declaration_func(const std::vector<SymbolPtr> &symbols);

  // 文法 7, 8, 39
  // T -> int
  // T -> void
  // T -> float
  Syntax::SymbolPtr reduce_type(const std::vector<SymbolPtr> &symbols);

  // 文法 9
  // A' -> ε
  Syntax::SymbolPtr reduce_params(const std::vector<SymbolPtr> &symbols);

  // 文法 10
  // A' -> A' A ;
  Syntax::SymbolPtr reduce_params_list(const std::vector<SymbolPtr> &symbols);

  // 文法 11
  // A -> T d
  Syntax::SymbolPtr reduce_param_var(const std::vector<SymbolPtr> &symbols);

  // 文法 12
  // A -> T d [ ]
  Syntax::SymbolPtr reduce_param_array(const std::vector<SymbolPtr> &symbols);

  // 文法 13
  // A -> T d ( )
  SyntaxZyl::SymbolPtr reduce_param_func(const std::vector<SymbolPtr> &symbols);

  // 文法 14
  // S' -> S
  SyntaxZyl::SymbolPtr reduce_sentences(const std::vector<SymbolPtr> &symbols);

  // 文法 15
  // S' -> S' ; S
  SyntaxZyl::SymbolPtr reduce_sentences_list(const std::vector<SymbolPtr> &symbols);

  // 文法 16
  // S -> d = E
  SyntaxZyl::SymbolPtr reduce_sentence_assign(const std::vector<SymbolPtr> &symbols);

  // 文法 17
  // S -> if ( B ) S
  SyntaxZyl::SymbolPtr reduce_sentence_if(const std::vector<SymbolPtr> &symbols);

  // 文法 18
  // S -> if ( B ) S else S
  SyntaxZyl::SymbolPtr reduce_sentence_if_else(const std::vector<SymbolPtr> &symbols);

  // 文法 19
  // S -> while ( B ) S
  SyntaxZyl::SymbolPtr reduce_sentence_while(const std::vector<SymbolPtr> &symbols);

  // 文法 20
  // S -> return E
  SyntaxZyl::SymbolPtr reduce_sentence_return(const std::vector<SymbolPtr> &symbols);

  // 文法 21
  // S -> { S' }
  SyntaxZyl::SymbolPtr reduce_sentence_block(const std::vector<SymbolPtr> &symbols);

  // 文法 22
  // S -> d ( R' )
  SyntaxZyl::SymbolPtr reduce_sentence_call(const std::vector<SymbolPtr> &symbols);

  // 文法 23
  // B -> B ∧ B
  SyntaxZyl::SymbolPtr reduce_bool_and(const std::vector<SymbolPtr> &symbols);

  // 文法 24
  // B -> B ∨ B
  SyntaxZyl::SymbolPtr reduce_bool_or(const std::vector<SymbolPtr> &symbols);

  // 文法 25
  // B -> E r E
  SyntaxZyl::SymbolPtr reduce_bool_relation(const std::vector<SymbolPtr> &symbols);

  // 文法 26
  // B -> E
  SyntaxZyl::SymbolPtr reduce_bool_expr(const std::vector<SymbolPtr> &symbols);

  // 文法 27
  // E -> d = E
  SyntaxZyl::SymbolPtr reduce_expr_assgin(const std::vector<SymbolPtr> &symbols);

  // 文法 28
  // E -> i
  SyntaxZyl::SymbolPtr reduce_expr_num_int(const std::vector<SymbolPtr> &syms);

  // 文法 29
  // E -> d
  SyntaxZyl::SymbolPtr reduce_expr_var(const std::vector<SymbolPtr> &symbols);

  // 文法 30
  // E -> d ( R' )
  SyntaxZyl::SymbolPtr reduce_expr_call(const std::vector<SymbolPtr> &syms);

  // 文法 31, 32, 46, 47
  // E -> E + E
  // E -> E * E
  // E -> E - E
  // E -> E / E
  SyntaxZyl::SymbolPtr reduce_expr_operator(const std::vector<SymbolPtr> &syms);

  // 文法 33
  // E -> ( E )
  SyntaxZyl::SymbolPtr reduce_expr_bracket(const std::vector<SymbolPtr> &syms);

  // 文法 34
  // R' -> ε
  SyntaxZyl::SymbolPtr reduce_call_params(const std::vector<SymbolPtr> &symbols);

  // 文法 35
  // R' -> R' R ,
  SyntaxZyl::SymbolPtr reduce_call_params_list(const std::vector<SymbolPtr> &symbols);

  // 文法 36
  // R -> E
  SyntaxZyl::SymbolPtr reduce_call_param_expr(const std::vector<SymbolPtr> &symbols);

  // 文法 37
  // R -> d [ ]
  SyntaxZyl::SymbolPtr reduce_call_param_array(const std::vector<SymbolPtr> &symbols);

  // 文法 38
  // R -> d ( )
  SyntaxZyl::SymbolPtr reduce_call_param_func(const std::vector<SymbolPtr> &symbols);

  // 文法 40
  // S -> d [ E ] = E
  SyntaxZyl::SymbolPtr reduce_sentence_array_assgin(const std::vector<SymbolPtr> &syms);

  // 文法 41
  // S -> for ( S ; B ; S ) S
  SyntaxZyl::SymbolPtr reduce_sentence_for(const std::vector<SymbolPtr> &syms);

  // 文法 42
  // S -> print E
  SyntaxZyl::SymbolPtr reduce_sentence_print(const std::vector<SymbolPtr> &syms);

  // 文法 43
  // S -> input d
  SyntaxZyl::SymbolPtr reduce_sentence_input(const std::vector<SymbolPtr> &syms);

  // 文法 44
  // E -> f
  SyntaxZyl::SymbolPtr reduce_expr_num_float(const std::vector<SymbolPtr> &syms);

  // 文法 45
  // E = d [ E ]
  SyntaxZyl::SymbolPtr reduce_expr_array(const std::vector<SymbolPtr> &syms);

  // 辅助函数


  std::string new_var();

  std::string new_label();

  std::string gen_code(const std::string& instruction, const std::string& label);

  std::string gen_code(const std::string& instruction, const std::vector<std::string> &labels);

  std::string new_labels(const std::vector<std::string> &labels);

  std::string new_params(const std::vector<std::string> &parlist);

  std::vector<std::string> merge_list(const std::vector<std::string> &list1, const std::vector<std::string> &list2);

  std::string merge_code(const std::vector<std::string> &code_list);

  EntryPtr check_var(std::string d);

  void add_equation(const Grammar& grammar, AttrEqnFuncPtr handler);

  static std::string type_convert(const std::string& type1, const std::string& type2);

  static std::string compute_const_number(const EPtr &e1, EPtr &e2, const std::string& op);

  static int size_of(const std::string& type);

  static bool is_array(EntryPtr &entry);

  static bool is_func(EntryPtr &entry);

  std::shared_ptr<Table> pop_symbol_table();

};


#endif // SYMBOL_TABLE_HPP
