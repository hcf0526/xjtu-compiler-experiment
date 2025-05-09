//
// Created by WangLele on 25-5-6.
//

#ifndef COMPILER_SIGNAL_HPP
#define COMPILER_SIGNAL_HPP
#include <QFile>
#include <string>

using Text = const std::string&;
using QText = const QString&;

inline QString filename_default_grammar = ":/resource/grammar.txt";

inline QFile file_default_lexical(":/resource/lexical.json");
inline QFile file_default_grammar(":/resource/grammar.txt");
inline QFile file_default_slr_table(":/resource/slr_table.csv");

bool parse_syntax(Text file, Text result);

bool parse_slr_table(QText filename_grammar, QText result);

#endif //COMPILER_SIGNAL_HPP
