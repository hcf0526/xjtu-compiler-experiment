//
// Created by WangLele on 25-5-6.
//

#ifndef COMPILER_WINDOW_HPP
#define COMPILER_WINDOW_HPP
#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class QTextEdit;
class QGridLayout;

class CompilerWindow : public QWidget {
  Q_OBJECT

public:
  explicit CompilerWindow(QWidget *parent = nullptr);
};

#endif //COMPILER_WINDOW_HPP
