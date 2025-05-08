#ifndef ITEM_HPP
#define ITEM_HPP

#include "grammar.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <iostream>

// 单个项目（Item）
class Item {
public:
  // 默认构造函数
  Item();

  // 从 Grammar 生成 Item，并插入点位
  Item(const Grammar &grammar, size_t pos);

  // 获取左部
  const std::string &lhs() const;

  // 获取右部
  const std::vector<std::string> &rhs() const;

  // 判断当前项目是否完成（点是否在末尾）
  bool completed() const;

  // 转换为对应的Grammar（去掉点）
  Grammar to_grammar() const;

  // 转为 JSON 表示
  json to_json() const;

  // 判断两个 Item 是否相同
  bool operator==(const Item &other) const;

  // 支持打印输出
  friend std::ostream &operator<<(std::ostream &os, const Item &item);

private:
  std::string lhs_; // 左部符号
  std::vector<std::string> rhs_; // 右部符号序列（带点）

  // 在指定位置插入点
  static std::vector<std::string> insert_dot(const std::vector<std::string> &rhs, size_t pos);
};

// 项目集（ItemSet）
class ItemSet {
public:
  // 默认构造函数
  ItemSet();

  // 从 Grammar 和 dot 位置构造
  ItemSet(const Grammar& grammar, size_t pos);

  // 从 GrammarSet 构造（所有产生式，点在最前）
  ItemSet(const GrammarSet& grammar_set, size_t pos = 0);

  // 从单个 Item 构造
  ItemSet(const Item& item);

  // 添加一个 Item
  void add(const Item &item);

  // 从 Grammar 添加一个 Item
  void add(const Grammar &grammar, size_t pos);

  // 获取所有 Item
  const std::unordered_map<std::string, std::vector<Item>> &items() const;

  // 非 const 访问
  std::vector<Item> &operator[](const std::string &lhs);

  // const 访问
  const std::vector<Item> &operator[](const std::string &lhs) const;

  // 判断是否包含某个 Item
  bool contains(const Item &item) const;

  // 判断ItemSet是否为空
  bool empty() const;

  // 所有Item的点向右移动一格，返回新的ItemSet
  ItemSet move() const;

  // 只移动点后是指定symbol的Item，返回新的ItemSet
  ItemSet move(const std::string& symbol) const;

  // 获取所有Item点后面的下一个符号集合
  std::unordered_set<std::string> next_symbols() const;

  // 求自身闭包
  ItemSet closure(const GrammarSet &grammar_set) const;

  // 求单个 Item 的闭包
  static ItemSet closure(const Item &item, const GrammarSet &grammar_set);

  // 求整个 ItemSet 的闭包
  static ItemSet closure(const ItemSet &item_set, const GrammarSet &grammar_set);

  // 求并集
  void union_set(const ItemSet &other);

  // 转为 JSON 对象
  json to_json() const;

  // 转为 JSON 并保存文件
  json to_json(const std::string &filename) const;

  // 支持打印输出
  friend std::ostream &operator<<(std::ostream &os, const ItemSet &item_set);

  // 判断两个 ItemSet 是否相同
  bool operator==(const ItemSet& other) const;

private:
  std::unordered_map<std::string, std::vector<Item>> items_; // 项目集，左部 -> 项列表
};

// 项目集簇（DFA的状态集合）
class ItemCluster {
public:
  // 单个状态（ItemSet）
  struct State {
    ItemSet kernel;  // 核（初始项目集）
    ItemSet closure; // 闭包（核扩展出的全部项目）
    std::unordered_map<std::string, std::string> goto_table; // 转移表：符号 -> 目标ItemSet名字
  };

  // 构造函数：传入文法集
  ItemCluster() = default;

  explicit ItemCluster(const GrammarSet& grammar_set);

  // 获取GrammarSet
  const GrammarSet& grammar_set() const;

  // 获取所有状态
  const std::unordered_map<std::string, State>& states() const;

  // 非 const 下标访问
  State& operator[](const std::string& state_name);

  // const 下标访问
  const State& operator[](const std::string& state_name) const;

  // 支持打印整个ItemCluster（项目集簇）
  friend std::ostream& operator<<(std::ostream& os, const ItemCluster& cluster);

  // 从文本解析
  void parse(const std::string &text);

  // 从文件解析
  void parse_file(const std::string &filename);

  // 构建ItemCluster
  void build();

  // 转为 JSON 对象
  json to_json() const;

  // 转为 JSON 并保存到文件
  void to_json(const std::string &filename) const;

  // 转为 txt 对象
  std::string to_txt() const;

  // 转为 txt 并保存到文件
  void to_txt(const std::string &filename) const;

  // 转为 dot 对象
  void to_dot(const std::string &filename) const;

private:
  std::unordered_map<std::string, State> states_; // 状态集，名称->State
  GrammarSet grammar_set_; // 当前使用的文法集
  int state_counter_; // 状态编号计数器，用于生成"Item Set N"

  // 从流解析
  void parse_stream(const std::string &content);
  // 生成新的状态名称（Item Set N）
  std::string generate_state_name();
  // 添加一个新的kernel（自动求closure）
  std::string add(const ItemSet& kernel);
  // 查找是否已有某个 ItemSet（根据 closure 比较）
  std::string find(const ItemSet& item_set) const;
  // 设置某个状态的转移
  void set_goto(const std::string& from_state, const std::string& symbol, const std::string& to_state);
};

#endif // ITEM_HPP
