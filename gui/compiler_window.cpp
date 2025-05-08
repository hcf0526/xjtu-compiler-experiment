//
// Created by WangLele on 25-5-5.
//

#include <fstream>
#include <iostream>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>


class CompilerWindow : public QWidget {
public:
  CompilerWindow(QWidget *parent = nullptr) : QWidget(parent) {
    setWindowTitle("编译器设计实验");
    this->setStyleSheet("font-size: 14pt;");
    setup_ui();
    setup_signal();
  }

  ~CompilerWindow() override = default;

private:
  // 程序选择行控件
  QLineEdit *program_line_edit;
  QPushButton *program_btn_browse;
  QPushButton *program_btn_view;
  QPushButton *program_btn_analyze;

  // 结果选择行控件
  QLineEdit *result_line_edit;
  QPushButton *result_btn_browse;

  // 查看内容区域控件（保持原名）
  QLabel *filename_label;
  QTextEdit *file_view;
  QWidget *view_widget;

  // 状态变量
  bool is_expanded = false;

  void setup_ui() {
    // 标题区
    QLabel *title = new QLabel("编译器设计实验");
    QLabel *subtitle = new QLabel("计算机2203 黄畅飞 2223515407");
    title->setAlignment(Qt::AlignCenter);
    subtitle->setAlignment(Qt::AlignCenter);
    QVBoxLayout *left_layout = new QVBoxLayout;
    left_layout->addWidget(title);
    left_layout->addWidget(subtitle);

    // 程序选择区
    QLabel *label_program = new QLabel("程序");
    program_line_edit = new QLineEdit;
    program_btn_browse = new QPushButton("选择");
    program_btn_view = new QPushButton("查看");
    program_btn_analyze = new QPushButton("分析");

    QHBoxLayout *program_input_layout = new QHBoxLayout;
    program_input_layout->addWidget(program_line_edit);
    program_input_layout->addWidget(program_btn_browse);
    program_input_layout->addWidget(program_btn_analyze);
    program_input_layout->addWidget(program_btn_view);

    QHBoxLayout *program_layout = new QHBoxLayout;
    program_layout->addWidget(label_program);
    program_layout->addLayout(program_input_layout);

    left_layout->addLayout(program_layout);

    // 结果选择区
    QLabel *label_result = new QLabel("结果");
    result_line_edit = new QLineEdit;
    result_btn_browse = new QPushButton("选择");

    QHBoxLayout *result_input_layout = new QHBoxLayout;
    result_input_layout->addWidget(result_line_edit);
    result_input_layout->addWidget(result_btn_browse);

    QHBoxLayout *result_layout = new QHBoxLayout;
    result_layout->addWidget(label_result);
    result_layout->addLayout(result_input_layout);

    left_layout->addLayout(result_layout);

    // 左布局
    left_layout->addStretch();

    QWidget *left_widget = new QWidget;
    left_widget->setLayout(left_layout);

    // 查看内容区
    filename_label = new QLabel;
    filename_label->setVisible(false);

    file_view = new QTextEdit;
    file_view->setReadOnly(true);
    file_view->setVisible(false);

    QVBoxLayout *view_layout = new QVBoxLayout;
    view_layout->addWidget(filename_label);
    view_layout->addWidget(file_view);

    view_widget = new QWidget;
    view_widget->setLayout(view_layout);
    view_widget->setVisible(false);

    // 主布局
    QGridLayout *main_layout = new QGridLayout;
    main_layout->addWidget(left_widget, 0, 0, 3, 1);
    main_layout->addWidget(view_widget, 0, 1, 3, 1);

    setLayout(main_layout);
  }

  void setup_signal() {
    connect(program_btn_browse, &QPushButton::clicked, this, [this]() {
      QString filename = QFileDialog::getOpenFileName(this, "选择程序文件");
      if (!filename.isEmpty()) {
        program_line_edit->setText(filename);
      }
    });

    connect(program_btn_view, &QPushButton::clicked, this, [this]() {
      if (!is_expanded) {
        QString filename = program_line_edit->text();
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          QTextStream in(&file);
          file_view->setText(in.readAll());

          QFileInfo info(filename);
          filename_label->setText("程序 " + info.fileName());
        } else {
          file_view->setText("无法打开文件");
          filename_label->setText("程序");
        }
        filename_label->setVisible(true);
        file_view->setVisible(true);
        view_widget->setVisible(true);
        program_btn_view->setText("隐藏");
        is_expanded = true;
      } else {
        view_widget->setVisible(false);
        program_btn_view->setText("查看");
        is_expanded = false;
      }
    });

    connect(program_btn_analyze, &QPushButton::clicked, this, [this]() {
      QString filename = program_line_edit->text();
      if (filename.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择程序文件");
        return;
      }
      // TODO: 分析逻辑
      QMessageBox::information(this, "分析", "分析功能未实现");
    });

    connect(result_btn_browse, &QPushButton::clicked, this, [this]() {
      QString dir = QFileDialog::getExistingDirectory(this, "选择结果文件夹");
      if (!dir.isEmpty()) {
        result_line_edit->setText(dir);
      }
    });
  }
};


int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  CompilerWindow window;
  window.resize(800, 300);
  window.show();

  return app.exec();
}
