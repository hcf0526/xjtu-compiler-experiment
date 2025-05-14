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
  struct SymbolTable;

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

  struct FuncEntry : Entry {
    std::shared_ptr<SymbolTable> mytab;
    friend std::ostream &operator<<(std::ostream &os, const FuncEntry &entry);
  };

  struct ArrayPttEntry: Entry {
    std::string etype;
    int base;
    friend std::ostream &operator<<(std::ostream &os, const ArrayPttEntry &entry);
  };

  struct FuncPttEntry: Entry {
    std::string rtype;
    friend std::ostream &operator<<(std::ostream &os, const FuncPttEntry &entry);
  };

  using EntryPtr = std::shared_ptr<struct Syntax::Entry>;
  using ArrayEntryPtr = std::shared_ptr<struct Syntax::ArrayEntry>;
  using FuncEntryPtr = std::shared_ptr<struct Syntax::FuncEntry>;
  using ArrayPttEntryPtr = std::shared_ptr<struct ArrayPttEntry>;
  using FuncPttEntryPtr = std::shared_ptr<struct FuncPttEntry>;

  struct SymbolTable {
    std::string name; // 符号表名称
    std::shared_ptr<SymbolTable> outer = nullptr; // 指向外层符号表
    int width = 0; // 符号表宽度
    int argc = 0; // 参数个数
    std::vector<std::string> arglist; // 参数名称列表
    std::string rtype; // 返回值类型
    int level = 0; // 层级
    std::vector<std::string> code; // 代码
    std::vector<std::shared_ptr<Entry> > entries; // 登记项

    explicit SymbolTable(std::string name = "") : name(std::move(name)) {} // 构造函数
    void add_entry(const std::shared_ptr<Entry> &entry); // 添加登记项
    friend std::ostream &operator<<(std::ostream &os, const SymbolTable &symbol_table);
  };

  using TablePtr = std::shared_ptr<struct Syntax::SymbolTable>;

  struct Process {
    int state; // 状态
    Lexical::Token token; // token
    SLRTable::Action action; // 动作
    friend std::ostream &operator<<(std::ostream &os, const Process &p);
  };

  struct SymbolWithAttribute {
    std::string name;
    std::string value;
    SymbolWithAttribute(std::string name, std::string value) : name(std::move(name)), value(std::move(value)) {}
    SymbolWithAttribute(const Lexical::Token& token) : name(token.type), value(token.lexeme) {}
    friend std::ostream &operator<<(std::ostream &os, const SymbolWithAttribute &symbol);
    virtual ~SymbolWithAttribute() = default;
  };

  using SymbolPtr = std::shared_ptr<struct SymbolWithAttribute>;

  // 构造函数：SLRTable作为语法分析器
  explicit Syntax(const SLRTable &slr_table);

  // 析构函数
  virtual ~Syntax() = default;

  // 解析输入字符串或文件
  bool parse(const std::vector<Lexical::Token> &tokens);

  bool parse_file(const std::string &filename, Lexical lexical);

  friend std::ostream &operator<<(std::ostream &os, const Syntax &analyzer);

  std::unordered_map<std::string, TablePtr> symbol_table() const;
  void processes_to_txt(const std::string &filename) const;
  void symbol_table_to_txt(const std::string &filename) const;

  static std::string get_table_name(const SymbolTable& symbol_table);

protected:
  const SLRTable &slr_table_; // 引用外部的语法分析表
  std::vector<Process> processes_; // 语法分析过程记录
  std::unordered_map<std::string, TablePtr> map_symbol_table_; // 符号表映射
  std::stack<std::shared_ptr<SymbolTable> > stack_symbol_table_; // 符号表栈

  // 辅助函数：从Token序列进行语义分析
  bool analyze_tokens(const std::vector<Lexical::Token> &tokens);

  virtual bool shift(int state_id, const Lexical::Token &token) = 0;

  virtual SymbolPtr reduce(int grammar_id, const std::vector<SymbolPtr> &symbols) = 0;
};

class SyntaxZyl : public Syntax {
public:
  struct SymbolT: SymbolWithAttribute {
    std::string type;
    SymbolT(std::string name, std::string value, std::string type)
        : SymbolWithAttribute(std::move(name), std::move(value)), type(std::move(type)) {}
  };
  using SymbolTPtr = std::shared_ptr<struct SymbolT>;

  struct SymbolD: SymbolWithAttribute {
    std::vector<std::string> place;
    SymbolD(std::string name, std::string value, std::vector<std::string> place)
        : SymbolWithAttribute(std::move(name), std::move(value)), place(std::move(place)) {}
  };
  using SymbolDPtr = std::shared_ptr<struct SymbolD>;

  struct SymbolA: SymbolWithAttribute {
    std::vector<std::string> place;
    SymbolA(std::string name, std::string value, std::vector<std::string> place)
        : SymbolWithAttribute(std::move(name), std::move(value)), place(std::move(place)) {}
  };
  using SymbolAPtr = std::shared_ptr<struct SymbolA>;

  struct SymbolAC: SymbolWithAttribute {
    std::vector<std::string> place;
    SymbolAC(std::string name, std::string value, std::vector<std::string> place)
        : SymbolWithAttribute(std::move(name), std::move(value)), place(std::move(place)) {}
  };
  using SymbolACPtr = std::shared_ptr<struct SymbolAC>;

  struct SymbolB: SymbolWithAttribute {
    std::vector<std::string> tc;
    std::vector<std::string> fc;
    std::string code;
    SymbolB(std::string name, std::string value, std::vector<std::string> tc, std::vector<std::string> fc, std::string code)
        : SymbolWithAttribute(std::move(name), std::move(value)), tc(std::move(tc)), fc(std::move(fc)), code(std::move(code)) {}
  };
  using SymbolBPtr = std::shared_ptr<struct SymbolB>;

  struct SymbolE: SymbolWithAttribute {
    std::string place;
    std::string code;
    SymbolE(std::string name, std::string value, std::string place, std::string code)
        : SymbolWithAttribute(std::move(name), std::move(value)), place(std::move(place)), code(std::move(code)) {}
  };
  using SymbolEPtr = std::shared_ptr<struct SymbolE>;

  struct SymbolEC: SymbolWithAttribute {
    std::vector<std::string> place;
    std::vector<std::string> code;
    SymbolEC(std::string name, std::string value, std::vector<std::string> place, std::vector<std::string> code)
        : SymbolWithAttribute(std::move(name), std::move(value)), place(std::move(place)), code(std::move(code)) {}
  };
  using SymbolECPtr = std::shared_ptr<struct SymbolEC>;

  struct SymbolP: SymbolWithAttribute {
    std::vector<std::string> code;
    SymbolP(std::string name, std::string value, std::vector<std::string> code)
        : SymbolWithAttribute(std::move(name), std::move(value)), code(std::move(code)) {}
  };
  using SymbolPPtr = std::shared_ptr<struct SymbolP>;

  struct SymbolR: SymbolWithAttribute {
    std::string code;
    std::string place;
    SymbolR(std::string name, std::string value, std::string code, std::string place)
        : SymbolWithAttribute(std::move(name), std::move(value)), code(std::move(code)), place(std::move(place)) {}
  };
  using SymbolRPtr = std::shared_ptr<struct SymbolR>;

  struct SymbolRC: SymbolWithAttribute {
    std::vector<std::string> place;
    std::vector<std::string> code;
    SymbolRC(std::string name, std::string value, std::vector<std::string> place, std::vector<std::string> code)
        : SymbolWithAttribute(std::move(name), std::move(value)), place(std::move(place)), code(std::move(code)) {}
  };
  using SymbolRCPtr = std::shared_ptr<struct SymbolRC>;

  struct SymbolS: SymbolWithAttribute {
    std::string code;
    SymbolS(std::string name, std::string value, std::string code)
        : SymbolWithAttribute(std::move(name), std::move(value)), code(std::move(code)) {}
  };
  using SymbolSPtr = std::shared_ptr<struct SymbolS>;

  struct SymbolSC: SymbolWithAttribute {
    std::vector<std::string> code;
    SymbolSC(std::string name, std::string value, std::vector<std::string> code)
        : SymbolWithAttribute(std::move(name), std::move(value)), code(std::move(code)) {}
  };
  using SymbolSCPtr = std::shared_ptr<struct SymbolSC>;

  explicit SyntaxZyl(const SLRTable &slr_table);

private:
  unsigned int variable_count_ = 0; // 变量计数器
  unsigned int label_count_ = 0; // 标签计数器
  using AttrEqnPtr = SymbolPtr (SyntaxZyl::*)(const std::vector<SymbolPtr>&);
  std::unordered_map<Grammar, AttrEqnPtr, Grammar::Hash> reduce_map_;

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

  // 文法 7, 8
  // T -> int
  // T -> void
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
  SyntaxZyl::SymbolPtr reduce_senteces(const std::vector<SymbolPtr> &symbols);

  // 文法 15
  // S' -> S' ; S
  SyntaxZyl::SymbolPtr reduce_senteces_list(const std::vector<SymbolPtr> &symbols);

  // 文法 16
  // S -> d = E
  SyntaxZyl::SymbolPtr reduce_sentece_assign(const std::vector<SymbolPtr> &symbols);

  // 文法 17
  // S -> if ( B ) S
  SyntaxZyl::SymbolPtr reduce_sentece_if(const std::vector<SymbolPtr> &symbols);

  // 文法 18
  // S -> if ( B ) S else S
  SyntaxZyl::SymbolPtr reduce_sentece_if_else(const std::vector<SymbolPtr> &symbols);

  // 文法 19
  // S -> while ( B ) S
  SyntaxZyl::SymbolPtr reduce_sentece_while(const std::vector<SymbolPtr> &symbols);

  // 文法 20
  // S -> return E
  SyntaxZyl::SymbolPtr reduce_sentece_return(const std::vector<SymbolPtr> &symbols);

  // 文法 21
  // S -> { S' }
  SyntaxZyl::SymbolPtr reduce_sentece_block(const std::vector<SymbolPtr> &symbols);

  // 文法 22
  // S -> d ( R' )
  SyntaxZyl::SymbolPtr reduce_sentece_call(const std::vector<SymbolPtr> &symbols);

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
  SyntaxZyl::SymbolPtr reduce_expr_num(const std::vector<SymbolPtr> &symbols);

  // 文法 29
  // E -> d
  SyntaxZyl::SymbolPtr reduce_expr_var(const std::vector<SymbolPtr> &symbols);

  // 文法 30
  // E -> d ( R' )
  SyntaxZyl::SymbolPtr reduce_expr_call(const std::vector<SymbolPtr> &symbols);

  // 文法 31
  // E -> E + E
  SyntaxZyl::SymbolPtr reduce_expr_add(const std::vector<SymbolPtr> &symbols);

  // 文法 32
  // E -> E * E
  SyntaxZyl::SymbolPtr reduce_expr_mul(const std::vector<SymbolPtr> &symbols);

  // 文法 33
  // E -> ( E )
  SyntaxZyl::SymbolPtr reduce_expr_bracket(const std::vector<SymbolPtr> &symbols);

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

  // 辅助函数
  std::string new_var();

  std::string new_label();

  std::string new_labels(const std::vector<std::string> &labels);

  std::string new_params(const std::vector<std::string> &parlist);

  std::string merge_code(const std::vector<std::string> &code_list);

  void add_equation(const Grammar& grammar, AttrEqnPtr handler);

  static int size_of(const std::string& type);

  std::shared_ptr<SymbolTable> pop_symbol_table();

};


#endif // SYMBOL_TABLE_HPP
