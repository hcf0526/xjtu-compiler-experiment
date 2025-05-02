#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <optional>

#include "slr_table.hpp"
#include "lexical.hpp"

class SymbolTableAnalyzer {
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

  using TablePtr = std::shared_ptr<struct SymbolTableAnalyzer::SymbolTable>;
  using EntryPtr = std::shared_ptr<struct SymbolTableAnalyzer::Entry>;
  using ArrayEntryPtr = std::shared_ptr<struct SymbolTableAnalyzer::ArrayEntry>;
  using FuncEntryPtr = std::shared_ptr<struct SymbolTableAnalyzer::FuncEntry>;

  struct Process {
    int state; // 状态
    Lexical::Token token; // token
    SLRTable::Action action; // 动作
    friend std::ostream &operator<<(std::ostream &os, const Process &p);
  };

  // 构造函数：SLRTable作为语法分析器
  explicit SymbolTableAnalyzer(const SLRTable &slr_table);

  // 析构函数
  virtual ~SymbolTableAnalyzer() = default;

  // 解析输入字符串或文件
  bool parse(const std::vector<Lexical::Token> &tokens);

  bool parse_file(const std::string &filename, Lexical lexical);

  friend std::ostream &operator<<(std::ostream &os, const SymbolTableAnalyzer &analyzer);

  void to_txt(const std::string &filename) const;

  static std::string get_table_name(const SymbolTable& symbol_table);

protected:
  const SLRTable &slr_table_; // 引用外部的语法分析表（只读）
  std::stack<std::shared_ptr<SymbolTable> > stack_symbol_table_; // 符号表栈
  std::vector<Process> processes_; // 语法分析过程记录
  std::unordered_map<std::string, TablePtr> map_symbol_table_; // 符号表映射

  // 辅助函数：从Token序列进行语义分析
  static const std::string &get_name(const Lexical::Token &token);

  bool analyze_tokens(const std::vector<Lexical::Token> &tokens);

  virtual bool shift(int state_id, const Lexical::Token &token) = 0;

  virtual bool reduce(int grammar_id, const std::vector<Lexical::Token> &tokens) = 0;
};

class SymbolTableAnalyzer1 : public SymbolTableAnalyzer {
public:
  explicit SymbolTableAnalyzer1(const SLRTable &slr_table);

  std::unordered_map<std::string, TablePtr> symbol_table() const;

private:
  // 移进/归约
  int temp = 0;

  bool reduce_var_declaration(const std::vector<Lexical::Token> &tokens);

  bool reduce_func_declaration(const std::vector<Lexical::Token> &tokens);

  bool reduce_func_params(const std::vector<Lexical::Token> &tokens);

  bool shift(int state_id, const Lexical::Token &token) override;

  bool reduce(int grammar_id, const std::vector<Lexical::Token> &tokens) override;

  // 辅助函数
  void push_symbol_table(const std::shared_ptr<SymbolTable>& symbol_table);

  std::shared_ptr<SymbolTable> pop_symbol_table();


};


#endif // SYMBOL_TABLE_HPP
