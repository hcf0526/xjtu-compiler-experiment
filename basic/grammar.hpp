#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

#include "utils/json.hpp"
#include "utils/format.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

// 单条文法（产生式）
class Grammar {
public:
  // 默认构造函数
  Grammar();

  // 带左部和右部的构造函数
  Grammar(const std::string& lhs, const std::vector<std::string>& rhs);

  // 文法字符串的构造函数（如 "A -> B C D"）
  explicit Grammar(const std::string& grammar_text);

  // 获取左部符号
  const std::string& lhs() const;

  // 获取右部符号序列
  const std::vector<std::string>& rhs() const;

  // 转为 JSON 表示
  json to_json() const;

  // 判断两个文法是否相同
  bool operator==(const std::string & other) const;
  bool operator==(const Grammar& other) const;

  // 支持打印输出
  friend std::ostream& operator<<(std::ostream& os, const Grammar& grammar);

  struct Hash {
    std::size_t operator()(const Grammar& g) const {
      std::size_t h1 = std::hash<std::string>{}(g.lhs());
      std::size_t h2 = 0;
      for (const auto& sym : g.rhs()) {
        h2 ^= std::hash<std::string>{}(sym) + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
      }
      return h1 ^ (h2 << 1);
    }
  };

private:
  std::string lhs_; // 左部非终结符
  std::vector<std::string> rhs_; // 右部符号列表
};

// 文法集合（多个产生式）
class GrammarSet {
public:
  // 默认构造函数
  GrammarSet();

  // 从文件构造文法
  explicit GrammarSet(const std::string& source);

  // 从文件构造文法(带起始符)
  GrammarSet(const std::string& source, const std::string& start_symbol);

  // 添加一条 Grammar
  void add(const Grammar& grammar);

  // 从一行文法字符串添加
  void add(const std::string& grammar_text);

  // 从文本解析文法
  bool parse(const std::string& grammar_text);

  // 从文件解析文法
  bool parse_file(const std::string& filename);

  // 从文本解析文法(带起始符)
  bool parse(const std::string& grammar_text, const std::string& start_symbol);

  // 从文件解析文法(带起始符)
  bool parse_file(const std::string& filename, const std::string& start_symbol);

  // 获取所有文法集合
  const std::unordered_map<std::string, std::vector<Grammar>>& grammars() const;

  // 非 const 下标访问
  std::vector<Grammar>& operator[](const std::string& lhs);

  // const 下标访问
  const std::vector<Grammar>& operator[](const std::string& lhs) const;

  // 获取起始符号
  const std::string& start_symbol() const { return start_; }

  // 获取所有终结符
  const std::unordered_set<std::string>& terminals() const { return terminals_; }

  // 获取所有非终结符
  const std::unordered_set<std::string>& non_terminals() const { return non_terminals_; }

  // 获取FIRST集合
  const std::unordered_map<std::string, std::unordered_set<std::string>>& first_set() const { return first_; }

  // 获取FOLLOW集合
  const std::unordered_map<std::string, std::unordered_set<std::string>>& follow_set() const { return follow_; }

  // 计算终结符和变元
  void compute_symbols();

  // 计算单个符号的FIRST
  std::unordered_set<std::string> compute_first(const std::string& symbol);

  // 计算所有变元的FIRST
  std::unordered_map<std::string, std::unordered_set<std::string>> compute_first();

  // 计算所有非终结符的 FOLLOW
  std::unordered_map<std::string, std::unordered_set<std::string>> compute_follow();

  // 设置起始符号
  void set_start(const std::string& start_symbol);

  // 转为 JSON 对象
  json to_json() const;

  // 转为 JSON 并保存到文件
  json to_json(const std::string& filename) const;

  // 支持打印输出
  friend std::ostream& operator<<(std::ostream& os, const GrammarSet& grammar_set);

private:
  std::string start_; // 起始文法符号
  std::unordered_set<std::string> terminals_; // 终结符集合
  std::unordered_set<std::string> non_terminals_; // 非终结符集合
  std::unordered_map<std::string, std::unordered_set<std::string>> first_; // FIRST 集合
  std::unordered_map<std::string, std::unordered_set<std::string>> follow_; // FOLLOW 集合
  std::unordered_map<std::string, std::vector<Grammar>> grammars_; // 文法表，左部 -> 产生式列表
  mutable std::unordered_set<std::string> in_progress_; // 计算 FIRST 时的中间状态

  // 辅助函数：从输入流解析文法
  bool parse_stream(std::istream& input);
};

#endif // GRAMMAR_HPP
