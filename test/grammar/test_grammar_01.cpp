#include "basic/grammar.hpp"
#include <string>

using json = nlohmann::json;

int main() {
  constexpr int index = 1;

  std::string input_filename = index_format("input/grammar/grammar", index, ".txt");
  std::string output_filename = index_format("output/grammar/grammar", index, ".json");

  GrammarSet grammar_set(input_filename);
  grammar_set.to_json(output_filename);
  return 0;
}
