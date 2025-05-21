#ifndef CSV_HPP
#define CSV_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class Csv {
public:
  Csv() = default;

  bool load(const std::string& text) {
    return load_stream(text);
  }

  // 从文件加载
  bool load_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return load_stream(buffer.str());
  }

  // 保存到文件
  bool save(const std::string& filename) const {
    // 用二进制模式打开，防止不同平台换行符混乱
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
      return false;
    }

    // 写入 UTF-8 BOM，避免Excel乱码
    file << "\xEF\xBB\xBF";

    // 写数据
    for (const auto& row : data_) {
      for (size_t i = 0; i < row.size(); ++i) {
        file << escape_field(row[i]);
        if (i != row.size() - 1) {
          file << ",";
        }
      }
      file << "\n";
    }

    file.close();
    return true;
  }

  // 添加一行
  void add_row(const std::vector<std::string>& row) {
    data_.push_back(row);
  }

  // 获取所有行
  const std::vector<std::vector<std::string>>& rows() const {
    return data_;
  }

private:
  std::vector<std::vector<std::string>> data_;

  static std::string escape_field(const std::string& field) {
    bool need_quotes = false;
    std::string escaped = field;

    if (field.find_first_of(",\"\n") != std::string::npos) {
      need_quotes = true;
    }

    if (field.find('"') != std::string::npos) {
      need_quotes = true;
      std::ostringstream oss;
      for (char ch : field) {
        if (ch == '"') {
          oss << "\"\""; // CSV标准：内部引号用两个引号表示
        } else {
          oss << ch;
        }
      }
      escaped = oss.str();
    }

    if (need_quotes) {
      return "\"" + escaped + "\"";
    } else {
      return escaped;
    }
  }

  bool load_stream(const std::string& content) {
    std::istringstream stream(content);
    data_.clear();
    std::string line;
    std::string current_line;
    int quote_count = 0;

    while (std::getline(stream, line)) {
      if (!current_line.empty()) current_line += "\n";
      current_line += line;
      // 统计当前行的引号数量
      quote_count += std::count(line.begin(), line.end(), '"');
      // 如果引号数量为偶数，说明字段闭合
      if (quote_count % 2 == 0) {
        data_.push_back(parse_csv_line(current_line));
        current_line.clear();
        quote_count = 0;
      }
    }
    // 处理最后一行（如果没有换行符结尾）
    if (!current_line.empty()) {
      data_.push_back(parse_csv_line(current_line));
    }
    return true;
  }

  std::vector<std::string> parse_csv_line(const std::string& line) {
    std::vector<std::string> result;
    std::string field;
    bool in_quotes = false;  // 标记是否在引号内

    for (size_t i = 0; i < line.size(); ++i) {
      char ch = line[i];
      if (in_quotes) {
        if (ch == '"') {
          // 如果遇到双引号，检查是否是转义的引号（两个引号）
          if (i + 1 < line.size() && line[i + 1] == '"') {
            field += '"';  // 处理双引号转义
            ++i;
          } else {
            in_quotes = false;  // 结束引号内的字段
          }
        } else {
          field += ch;  // 在引号内，继续添加字符
        }
      } else {
        if (ch == '"') {
          in_quotes = true;  // 进入引号内
        } else if (ch == ',') {
          result.push_back(field);  // 逗号分隔，保存当前字段
          field.clear();
        } else {
          field += ch;  // 普通字符，添加到字段
        }
      }
    }
    result.push_back(field);  // 最后一个字段
    return result;
  }
};

#endif // CSV_HPP
