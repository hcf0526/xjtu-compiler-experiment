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

Lexical::Lexical(const std::string& regex_config_path) {
  std::ifstream in(regex_config_path);
  if (!in) throw std::runtime_error("Failed to open regex config file.");
  json j;
  std::string temp;
  in >> j;

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
      regex_rules_.emplace_back(type, std::regex("^" + pattern));
    }
  }
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

void Lexical::write_tokens(const std::string& output_path) const {
  std::ofstream out(output_path);
  for (const auto& tok : tokens_) {
    if (tok.type == "NEWLINE") {
      out << "\n";
    } else {
      out << tok << " ";
    }
  }
}