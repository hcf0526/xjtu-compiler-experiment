#include "lexical.hpp"
#include "utils/json.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <filesystem>

using json = nlohmann::json;

std::ostream& operator<<(std::ostream& os, const Lexical::Token& tok) {
  os << "(" << tok.type << ", " << tok.lexeme << ")";
  return os;
}

Lexical::Lexical(const std::string& content) {
  if (content.find('{') != std::string::npos) {
    parse(content);
  } else {
    parse_file(content);
  }
}

void Lexical::parse(const std::string &text) {
  parse_stream(text);
}

void Lexical::parse_file(const std::string &file) {
  std::ifstream in(file);
  if (!in) {
    std::cerr << "[Lexical] 无法打开文件" << file << std::endl;
    return;
  }

  std::stringstream buffer;
  buffer << in.rdbuf();
  in.close();
  parse_stream(buffer.str());
}

std::vector<Lexical::Token> Lexical::analyze(const std::string& input) {
  std::string source;
  std::ifstream file(input);
  if (file.good()) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    source = buffer.str();
  } else {
    source = input;
  }

  tokens_.clear();
  std::regex skip_ws(R"([ \t\r]+)");
  size_t pos = 0;
  while (pos < source.length()) {
    std::smatch match;
    if (std::regex_search(source.cbegin() + pos, source.cend(), match, skip_ws) && match.position() == 0) {
      pos += match.length();
      continue;
    }

    bool matched = false;
    for (const auto& [type, regex] : regex_rules_) {
      if (std::regex_search(source.cbegin() + pos, source.cend(), match, regex)) {
        tokens_.push_back({type, match.str()});
        pos += match.length();
        matched = true;
        break;
      }
    }
    if (!matched) {
      tokens_.push_back({"UNKNOWN", std::string(1, source[pos])});
      ++pos;
    }
  }

  return tokens_;
}

void Lexical::to_txt(const std::string& output_path) const {
  std::ofstream out(output_path);
  for (const auto& tok : tokens_) {
    if (tok.type == "NEWLINE") {
      out << "\n";
    } else {
      out << tok << " ";
    }
  }
}

void Lexical::parse_stream(const std::string &file) {
  json j;
  try {
    j = json::parse(file);
  } catch (const std::exception& e) {
    std::cerr << "[Lexical] json 解析错误: " << e.what() << std::endl;
    return;
  }

  regex_rules_.clear();
  std::vector<std::pair<int, std::vector<std::pair<std::string, std::string>>>> sorted;

  for (auto& [priority_str, rules] : j.items()) {
    int pri = std::stoi(priority_str);
    std::vector<std::pair<std::string, std::string>> group;
    for (auto& [type, regex_str] : rules.items()) {
      group.emplace_back(type, regex_str);
    }
    sorted.emplace_back(pri, group);
  }

  std::sort(sorted.begin(), sorted.end());

  for (auto& [_, group] : sorted) {
    for (auto& [type, pattern] : group) {
      try {
        regex_rules_.emplace_back(type, std::regex("^" + pattern));
      } catch (const std::regex_error& e) {
        std::cerr << "[Lexical] 错误的正则表达式. type: " << type << ", pattern: " << pattern
                  << std::endl << e.what() << std::endl;
        return;
      }
    }
  }
}
