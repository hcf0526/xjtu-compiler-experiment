#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <optional>

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
    std::string rtype; // 返回值类型
    std::shared_ptr<SymbolTable> mytab;
    friend std::ostream &operator<<(std::ostream &os, const FuncEntry &entry);
  };

  struct SymbolTable {
    std::string name; // 符号表名称
    std::shared_ptr<SymbolTable> outer = nullptr; // 指向外层符号表
    int width = 0; // 符号表宽度
    int argc = 0; // 参数个数
    std::vector<std::string> arglist; // 参数名称列表
    std::string rtype; // 返回值类型
    int level = 0; // 层级
    std::string code; // 代码
    std::vector<std::shared_ptr<Entry> > entries; // 登记项

    explicit SymbolTable(std::string name = "") : name(std::move(name)) {
    } // 构造函数
    void add_entry(const std::shared_ptr<Entry> &entry); // 添加登记项
    friend std::ostream &operator<<(std::ostream &os, const SymbolTable &st);
  };

  using TablePtr = std::shared_ptr<struct Syntax::SymbolTable>;
  using EntryPtr = std::shared_ptr<struct Syntax::Entry>;
  using ArrayEntryPtr = std::shared_ptr<struct Syntax::ArrayEntry>;
  using FuncEntryPtr = std::shared_ptr<struct Syntax::FuncEntry>;


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
    friend std::ostream &operator<<(std::ostream &os, const SymbolWithAttribute &swa);
    virtual ~SymbolWithAttribute() = default;
  };
  using SymAttrPtr = std::shared_ptr<struct SymbolWithAttribute>;

  // 构造函数：SLRTable作为语法分析器
  explicit Syntax(const SLRTable &slr_table);

  // 析构函数
  virtual ~Syntax() = default;

  // 解析输入字符串或文件
  bool parse(const std::vector<Lexical::Token> &tokens);

  bool parse_file(const std::string &filename, Lexical lexical);

  friend std::ostream &operator<<(std::ostream &os, const Syntax &analyzer);

  void processes_to_txt(const std::string &filename) const;
  void symbol_table_to_txt(const std::string &filename) const;

  static std::string get_name(const SymbolTable& symbol_table);

protected:
  const SLRTable &slr_table_; // 引用外部的语法分析表（只读）
  std::stack<std::shared_ptr<SymbolTable> > stack_symbol_table_; // 符号表栈
  std::vector<Process> processes_; // 语法分析过程记录
  std::unordered_map<std::string, TablePtr> map_symbol_table_; // 符号表映射

  // 辅助函数：从Token序列进行语义分析
  bool analyze_tokens(const std::vector<Lexical::Token> &tokens);

  virtual bool shift(int state_id, const Lexical::Token &token) = 0;

  virtual SymAttrPtr reduce(int grammar_id, const std::vector<SymAttrPtr> &symbols) = 0;
};

class SyntaxZyl : public Syntax {
public:
  struct ArrayPttEntry: Entry {
    std::string etype;
    int base;
    friend std::ostream &operator<<(std::ostream &os, const ArrayPttEntry &entry);
  };
  struct FuncPttEntry: Entry {
    std::string rtype;
    friend std::ostream &operator<<(std::ostream &os, const FuncPttEntry &entry);
  };
  using ArrayPttEntryPtr = std::shared_ptr<struct ArrayPttEntry>;
  using FuncPttEntryPtr = std::shared_ptr<struct FuncPttEntry>;
  struct SymbolT: SymbolWithAttribute {
    std::string type;
    SymbolT(std::string name, std::string value, std::string type)
        : SymbolWithAttribute(std::move(name), std::move(value)), type(std::move(type)) {}
    SymbolT(const Lexical::Token& token, std::string type)
        : SymbolWithAttribute(token), type(std::move(type)) {}
  };
  using SymbolTPtr = std::shared_ptr<struct SymbolT>;

  explicit SyntaxZyl(const SLRTable &slr_table);

  std::unordered_map<std::string, TablePtr> symbol_table() const;

  static int size_of(const std::string& type);

private:
  // 移进/归约

  // 文法 4
  // D -> T d
  Syntax::SymAttrPtr reduce_var_declaration(const std::vector<SymAttrPtr>& symbols);

  // 文法 5
  // D -> T d [ i ]
  Syntax::SymAttrPtr reduce_array_declaration(const std::vector<SymAttrPtr>& symbols);

  // 文法 6
  // D -> T d ( A' ) { D' S' }
  Syntax::SymAttrPtr reduce_func_declaration(const std::vector<SymAttrPtr>& symbols);

  // 文法 7, 8
  // T -> int
  // T -> void
  SyntaxZyl::SymbolTPtr reduce_type(const std::vector<SymAttrPtr>& symbols);

  // 文法 9
  // A' -> ε
  Syntax::SymAttrPtr reduce_params(const std::vector<SymAttrPtr>& symbols);

  // 文法 11
  // A -> T d
  Syntax::SymAttrPtr reduce_params_var(const std::vector<SymAttrPtr>& symbols);

  // 文法 12
  // A -> T d [ ]
  Syntax::SymAttrPtr reduce_params_array(const std::vector<SymAttrPtr>& symbols);

  // 文法 13
  // A -> T d ( )
  Syntax::SymAttrPtr reduce_params_func(const std::vector<SymAttrPtr>& symbols);

  bool shift(int state_id, const Lexical::Token &token) override;

  SymAttrPtr reduce(int grammar_id, const std::vector<SymAttrPtr> &symbols) override;

  // 辅助函数
  void push_symbol_table(const std::shared_ptr<SymbolTable>& symbol_table);

  std::shared_ptr<SymbolTable> pop_symbol_table();

};


#endif // SYMBOL_TABLE_HPP
