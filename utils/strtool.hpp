#ifndef STRTOOL_HPP
#define STRTOOL_HPP

#include <string>
#include <vector>
#include <sstream>

// 字符串工具函数集合
namespace strtool {

  // 去除字符串首尾空白字符（空格、Tab、回车、换行）
  inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
  }

  // 按分隔符切分字符串，返回子串数组
  inline std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(s);
    while (std::getline(token_stream, token, delimiter)) {
      tokens.push_back(trim(token)); // 每个切出来的词也顺便trim
    }
    return tokens;
  }

} // namespace strtool

#endif // STRTOOL_HPP
