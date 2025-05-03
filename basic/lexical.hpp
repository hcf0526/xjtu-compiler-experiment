#ifndef LEXICAL_HPP
#define LEXICAL_HPP

#include <string>
#include <vector>
#include <regex>
#include <ostream>


// 词法分析类：封装从 JSON 配置加载规则、分析输入、输出结果
class Lexical {
public:
  // 词法记号结构体，类型使用字符串
  struct Token {
    std::string type;     // 类型字符串，如 "ID", "NUM", "IF"
    std::string lexeme;   // 匹配到的原始文本

    // 支持输出 (type, lexeme)
    friend std::ostream& operator<<(std::ostream& os, const Token& tok);
  };

  // 构造函数：从 JSON 文件加载正则规则
  explicit Lexical(const std::string& regex_config_path);

  // 输入可以是源码字符串或文件路径，自动判断
  std::vector<Token> analyze(const std::string& input);

  // 输出所有 Token 到文件（NEWLINE 输出为换行符）
  void to_txt(const std::string& output_path) const;

private:
  std::vector<Token> tokens_;                                    // 词法分析结果
  std::vector<std::pair<std::string, std::regex>> regex_rules_;  // (type, regex)
};

#endif // LEXICAL_HPP
