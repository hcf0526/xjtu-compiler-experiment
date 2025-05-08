//
// Created by WangLele on 25-5-6.
//

#ifndef COMPILER_SIGNAL_HPP
#define COMPILER_SIGNAL_HPP
#include <QFile>
#include <string>

using Text = const std::string&;

inline QFile default_lexical(":/resource/lexical.json");
inline QFile default_grammar(":/resource/grammar.txt");
inline QFile default_slr_table(":/resource/slr_table.csv");

bool parse(Text program, Text result);

#endif //COMPILER_SIGNAL_HPP
