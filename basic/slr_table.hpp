#ifndef SLR_TABLE_HPP
#define SLR_TABLE_HPP

#include "item.hpp"
#include "grammar.hpp"
#include <unordered_map>
#include <string>
#include <iostream>


// SLR分析表
class SLRTable {
public:
  // 动作类型
  enum ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

  // 冲突类型
  enum ConflictType { SHIFT_REDUCE, REDUCE_REDUCE, SHIFT_SHIFT, UNKNOWN };

  struct Action {
    ActionType type;
    int target;

    bool operator==(const Action& other) const {
      return type == other.type && target == other.target;
    }

    bool operator<(const Action& other) const {
      return std::tie(type, target) < std::tie(other.type, other.target);
    }

    friend std::ostream& operator<<(std::ostream& os, const Action& action);
  };

  struct ActionHash {
    size_t operator()(const Action& a) const {
      return std::hash<int>()(a.type) ^ (std::hash<int>()(a.target) << 1);
    }
  };

  using ActionSet = std::unordered_set<Action, ActionHash>;
  using ActionRow = std::unordered_map<std::string, ActionSet>;

  using GotoSet = std::unordered_set<int>;
  using GotoRow = std::unordered_map<std::string, GotoSet>;

  // 冲突记录
  struct Conflict {
    int state;
    std::string symbol;
    ConflictType type;
    ActionSet actions;

    bool operator==(const Conflict& other) const {
      return state == other.state &&
             symbol == other.symbol &&
             type == other.type &&
             actions == other.actions;
    }

    friend std::ostream& operator<<(std::ostream& os, const Conflict& conflict);
  };

  struct ConflictHash {
    size_t operator()(const Conflict& c) const {
      size_t h1 = std::hash<int>()(c.state);
      size_t h2 = std::hash<std::string>()(c.symbol);
      size_t h3 = std::hash<int>()(static_cast<int>(c.type));

      size_t h4 = 0;
      for (const auto& act : c.actions) {
        h4 ^= ActionHash{}(act) + 0x9e3779b9 + (h4 << 6) + (h4 >> 2);
      }

      return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
  };

  using ConflictSet = std::unordered_set<Conflict, ConflictHash>;

  // 默认构造
  SLRTable() = default;

  // 从文本/文件构造
  explicit SLRTable(const std::string& content);

  // 从ItemCluster构造初始化
  explicit SLRTable(const ItemCluster& cluster);

  // 获取某个状态的在某个符号下的ACTION表
  const ActionSet* get_action(int state, const std::string& symbol) const;

  // 获取某个状态的ACTION表
  ActionRow get_action(int state) const;

  // 获取所有状态的ACTION表
  std::unordered_map<int, ActionRow> get_action() const;

  // 获取某个状态的在某个符号下GOTO表
  const GotoSet* get_goto(int state, const std::string& non_terminal) const;

  // 获取某个状态的GOTO表
  GotoRow get_goto(int state) const;

  // 获取所有状态的GOTO表
  std::unordered_map<int, GotoRow> get_goto() const;

  // 添加ACTION表项
  void add_action(int state, const std::string& symbol, ActionType type, int target);

  // 添加GOTO表项
  void add_goto(int state, const std::string& non_terminal, int next_state);

  // 获取起始/接受/最终接受/冲突状态
  int start_state() const;
  const std::unordered_set<int>& accept_states() const;
  int final_accept_state() const;
  const ConflictSet& conflicts() const;

  // 获取ItemSet/Grammar到编号的映射
  const std::unordered_map<std::string, int>& state_to_id() const;
  const std::unordered_map<Grammar, int, Grammar::Hash>& grammar_to_id() const;
  const std::unordered_map<int, std::string> id_to_state() const;
  const std::unordered_map<int, Grammar> id_to_grammar() const;

  // 找到id对应的状态名字
  std::string find_state(int id) const;
  int find_state(const std::string& name) const;

  // 找到id对应的项目集
  ItemCluster::State find_item_set(int id) const;

  // 找到id对应的Grammar
  Grammar find_grammar(int id) const;
  int find_grammar(const Grammar &grammar) const;

  // IMPORTANT: 核心
  // 使用类中的ItemCluster构建
  void build();

  // 使用指定的ItemCluster构建
  void build(const ItemCluster& cluster);

  // 保存CSV文件
  void to_csv(const std::string &filename) const;

  // 从文本加载
  void parse(const std::string& text);

  // 从文件加载
  void parse_file(const std::string& file);

  // 读取CSV文件
  void read_csv(const std::string& file);

private:
  ItemCluster item_cluster_; // 保存自己的ItemCluster

  // ACTION表：state -> (symbol -> 动作列表)
  std::unordered_map<int, ActionRow> action_table_;

  // GOTO表：state -> (非终结符 -> 目标状态)
  std::unordered_map<int, GotoRow> goto_table_;

  int start_state_;  // 起始状态编号
  std::unordered_set<int> accept_states_; // 所有接受状态编号集合
  int final_accept_state_; // 最终接受状态（S'->S的状态编号）
  ConflictSet conflicts_; // 冲突列表

  std::unordered_map<std::string, int> state_to_id_; // ItemSet名字 -> 状态编号
  std::unordered_map<Grammar, int, Grammar::Hash> grammar_to_id_; // Grammar -> 产生式编号
  std::unordered_map<int, std::string> id_to_state_; // 状态编号 -> ItemSet名字
  std::unordered_map<int, Grammar> id_to_grammar_; // 产生式编号 -> Grammar

  // 分配状态编号和产生式编号
  void assign_ids(const ItemCluster& cluster);

  // 计算起始状态、接受状态、最终接受状态
  void compute_states(const ItemCluster& cluster);

  // 计算冲突
  int compute_conflict();

  void parse_stream(const std::string& content);

  // 辅助函数：从 Item Set 名字中提取编号
  static int extract_number(const std::string& name);

  // 辅助函数：检测冲突类型
  static ConflictType detect_conflict_type(const SLRTable::ActionSet& actions);

};

#endif // SLR_TABLE_HPP
