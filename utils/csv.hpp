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

public:
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
    while (std::getline(stream, line)) {
      data_.push_back(parse_csv_line(line));
    }
    return true;
  }

  static std::vector<std::string> parse_csv_line(const std::string& line) {
    std::vector<std::string> result;
    std::string field;
    bool in_quotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
      char ch = line[i];
      if (in_quotes) {
        if (ch == '"') {
          if (i + 1 < line.size() && line[i + 1] == '"') {
            field += '"';
            ++i;
          } else {
            in_quotes = false;
          }
        } else {
          field += ch;
        }
      } else {
        if (ch == '"') {
          in_quotes = true;
        } else if (ch == ',') {
          result.push_back(field);
          field.clear();
        } else {
          field += ch;
        }
      }
    }
    result.push_back(field); // 最后一个字段
    return result;
  }
};

#endif // CSV_HPP
