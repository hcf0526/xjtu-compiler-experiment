//
// Created by WangLele on 25-5-5.
//

#include "compiler_signal.hpp"
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
#include <QButtonGroup>


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
  // 布局
  QVBoxLayout *left_top_layout;
  QVBoxLayout *title_layout;
  QGridLayout *program_layout;
  QGridLayout *result_layout;
  QHBoxLayout *analyze_layout;
  QHBoxLayout *more_btn_layout;
  QGridLayout *lexical_layout;
  QGridLayout *grammar_layout;
  QGridLayout *slr_table_layout;
  QVBoxLayout *left_bottom_layout;
  QVBoxLayout *left_layout;
  QVBoxLayout *view_layout;
  QGridLayout *main_layout;

  // 程序选择
  QLineEdit *program_line_edit;
  QPushButton *program_btn_browse;
  QPushButton *program_btn_view;

  // 结果选择
  QLineEdit *result_line_edit;
  QPushButton *result_btn_browse;

  // 分析操作
  QPushButton *analyze_btn_lexical_analyze;
  QPushButton *analyze_btn_item_cluster;
  QPushButton *analyze_btn_slr_table;
  QPushButton *analyze_btn_syntax_analyze;
  QPushButton *analyze_btn_assembly_code;
  QPushButton *analyze_btn_binary_code;

  // 更多按钮
  QPushButton *more_btn_toggle;
  QWidget *bottom_widget;

  // 词法选择
  QLineEdit *lexical_line_edit;
  QPushButton *lexical_btn_browse;
  QPushButton *lexical_btn_default;
  QPushButton *lexical_btn_view;

  // 文法选择
  QLineEdit *grammar_line_edit;
  QPushButton *grammar_btn_browse;
  QPushButton *grammar_btn_default;
  QPushButton *grammar_btn_view;

  // SLR表选择
  QLineEdit *slr_table_line_edit;
  QPushButton *slr_table_btn_browse;
  QPushButton *slr_table_btn_default;
  QPushButton *slr_table_btn_view;

  // 查看内容区域控件
  QLabel *filename_label;
  QTextEdit *file_view;
  QWidget *view_widget;

  // 查看按钮组
  QButtonGroup *view_btn_group;

  void setup_ui_layout() {
    left_top_layout = new QVBoxLayout;
    title_layout = new QVBoxLayout;
    program_layout = new QGridLayout;
    result_layout = new QGridLayout;
    analyze_layout = new QHBoxLayout;
    more_btn_layout = new QHBoxLayout;
    lexical_layout = new QGridLayout;
    grammar_layout = new QGridLayout;
    slr_table_layout = new QGridLayout;
    left_bottom_layout = new QVBoxLayout;
    left_layout = new QVBoxLayout;
    view_layout = new QVBoxLayout;
    main_layout = new QGridLayout;

    // 左上布局
    left_top_layout->addLayout(title_layout);
    left_top_layout->addLayout(program_layout);
    left_top_layout->addLayout(result_layout);
    left_top_layout->addLayout(analyze_layout);
    left_top_layout->addLayout(more_btn_layout);
    left_top_layout->addStretch();

    // 左下布局
    left_bottom_layout->addLayout(lexical_layout);
    left_bottom_layout->addLayout(grammar_layout);
    left_bottom_layout->addLayout(slr_table_layout);

    bottom_widget = new QWidget;
    bottom_widget->setLayout(left_bottom_layout);
    bottom_widget->setVisible(false); // 默认隐藏

    // 左布局
    left_layout->addLayout(left_top_layout);
    left_layout->addWidget(bottom_widget);
    left_layout->addStretch();

    QWidget *left_widget = new QWidget;
    left_widget->setLayout(left_layout);

    // 右布局
    view_widget = new QWidget;
    view_widget->setLayout(view_layout);
    view_widget->setVisible(false);

    // 主布局
    main_layout->addWidget(left_widget, 0, 0, 3, 1);
    main_layout->addWidget(view_widget, 0, 1, 3, 1);
    this->setLayout(main_layout);
  }

  void setup_ui_title() {
    QLabel *title = new QLabel("编译器设计实验");
    QLabel *subtitle = new QLabel("计算机2203 黄畅飞 2223515407");
    title->setAlignment(Qt::AlignCenter);
    subtitle->setAlignment(Qt::AlignCenter);

    title_layout->addWidget(title);
    title_layout->addWidget(subtitle);
  }

  void setup_ui_program() {
    QLabel *label_program = new QLabel("程序");
    program_line_edit = new QLineEdit;
    program_btn_browse = new QPushButton("选择");
    program_btn_view = new QPushButton("查看");

    program_layout->addWidget(label_program, 0, 0);
    program_layout->addWidget(program_line_edit, 0, 1);
    program_layout->addWidget(program_btn_browse, 0, 2);
    program_layout->addWidget(program_btn_view, 0, 3);
  }

  void setup_ui_result() {
    QLabel *label_result = new QLabel("结果");
    result_line_edit = new QLineEdit;
    result_line_edit->setText("D:/Github/xjtu-compiler-experiment/result"); // 暂时使用这个
    result_btn_browse = new QPushButton("选择");

    result_layout->addWidget(label_result,0, 1);
    result_layout->addWidget(result_line_edit, 0, 2);
    result_layout->addWidget(result_btn_browse, 0, 3);
  }

  void setup_ui_analyze() {
    analyze_btn_lexical_analyze = new QPushButton("词法分析");
    analyze_btn_item_cluster = new QPushButton("项目集簇");
    analyze_btn_slr_table = new QPushButton("SLR分析表");
    analyze_btn_syntax_analyze = new QPushButton("语法分析");
    analyze_btn_assembly_code = new QPushButton("汇编代码");
    analyze_btn_binary_code = new QPushButton("二进制代码");

    analyze_layout->addWidget(analyze_btn_lexical_analyze);
    analyze_layout->addWidget(analyze_btn_item_cluster);
    analyze_layout->addWidget(analyze_btn_slr_table);
    analyze_layout->addWidget(analyze_btn_syntax_analyze);
    analyze_layout->addWidget(analyze_btn_assembly_code);
    analyze_layout->addWidget(analyze_btn_binary_code);
  }

  void setup_ui_more() {
    more_btn_toggle = new QPushButton("更多");
    more_btn_toggle->setFixedSize(80, 30);

    more_btn_layout->addStretch(); // 靠右对齐
    more_btn_layout->addWidget(more_btn_toggle);
  }

  void setup_ui_lexical() {
    QLabel *lexical_label = new QLabel("词法文件");
    lexical_line_edit = new QLineEdit;
    lexical_line_edit->setText(":/resource/lexical.json");
    lexical_btn_browse = new QPushButton("选择");
    lexical_btn_default = new QPushButton("默认");
    lexical_btn_view = new QPushButton("查看");

    lexical_layout->addWidget(lexical_label, 0, 0);
    lexical_layout->addWidget(lexical_line_edit, 0, 1);
    lexical_layout->addWidget(lexical_btn_browse, 0, 2);
    lexical_layout->addWidget(lexical_btn_default, 0, 3);
    lexical_layout->addWidget(lexical_btn_view, 0, 4);
  }

  void setup_ui_grammar() {
    QLabel *grammar_label = new QLabel("文法文件");
    grammar_line_edit = new QLineEdit;
    grammar_line_edit->setText(":/resource/grammar.txt");
    grammar_btn_browse = new QPushButton("选择");
    grammar_btn_default = new QPushButton("默认");
    grammar_btn_view = new QPushButton("查看");

    grammar_layout->addWidget(grammar_label, 0, 0);
    grammar_layout->addWidget(grammar_line_edit, 0, 1);
    grammar_layout->addWidget(grammar_btn_browse, 0, 2);
    grammar_layout->addWidget(grammar_btn_default, 0, 3);
    grammar_layout->addWidget(grammar_btn_view, 0, 4);
  }

  void setup_ui_slr_table() {
    QLabel *slr_table_label = new QLabel("SLR表文件");
    slr_table_line_edit = new QLineEdit;
    slr_table_line_edit->setText(":/resource/slr_table.csv");
    slr_table_btn_browse = new QPushButton("选择");
    slr_table_btn_default = new QPushButton("默认");
    slr_table_btn_view = new QPushButton("查看");
    slr_table_layout->addWidget(slr_table_label, 0, 0);
    slr_table_layout->addWidget(slr_table_line_edit, 0, 1);
    slr_table_layout->addWidget(slr_table_btn_browse, 0, 2);
    slr_table_layout->addWidget(slr_table_btn_default, 0, 3);
    slr_table_layout->addWidget(slr_table_btn_view, 0, 4);
  }

  void setup_ui_view() {
    filename_label = new QLabel;
    filename_label->setVisible(false);

    file_view = new QTextEdit;
    file_view->setReadOnly(true);
    file_view->setVisible(false);

    view_layout->addWidget(filename_label);
    view_layout->addWidget(file_view);
  }

  void setup_ui() {
    setup_ui_layout();
    setup_ui_title();
    setup_ui_program();
    setup_ui_result();
    setup_ui_analyze();
    setup_ui_more();
    setup_ui_lexical();
    setup_ui_grammar();
    setup_ui_slr_table();
    setup_ui_view();
  }

  void setup_signal_browse() {
    // 浏览按钮
    QMap<QPushButton*, QPair<QLineEdit*, QString>> browse_map;
    browse_map.insert(program_btn_browse, { program_line_edit, "选择程序文件" });
    browse_map.insert(lexical_btn_browse, { lexical_line_edit, "选择词法文件" });
    browse_map.insert(grammar_btn_browse, { grammar_line_edit, "选择文法文件" });
    browse_map.insert(slr_table_btn_browse, { slr_table_line_edit, "选择SLR表文件" });
    for (auto it = browse_map.begin(); it != browse_map.end(); ++it) {
      QPushButton *btn = it.key();
      QLineEdit *line_edit = it.value().first;
      QString title = it.value().second;

      connect(btn, &QPushButton::clicked, this, [=]() {
        signal_browse_file(line_edit, title);
      });
    }
  }

  void setup_signal_default() {
    // 默认按钮
    QMap<QPushButton*, QPair<QLineEdit*, QString>> default_map;
    default_map.insert(lexical_btn_default, { lexical_line_edit, ":/resource/lexical.json" });
    default_map.insert(grammar_btn_default, { grammar_line_edit, ":/resource/grammar.txt" });
    default_map.insert(slr_table_btn_default, { slr_table_line_edit, ":/resource/slr_table.csv" });
    for (auto it = default_map.begin(); it != default_map.end(); ++it) {
      QPushButton *btn = it.key();
      QLineEdit *edit = it.value().first;
      QString path = it.value().second;

      connect(btn, &QPushButton::clicked, this, [=]() {
        edit->setText(path);
      });
    }
  }

  void setup_signal_analyze() {
    // 分析按钮
    QMap<QPushButton*, std::function<void()>> analyze_map;
    analyze_map.insert(analyze_btn_lexical_analyze, [this]() { signal_lexical_analysis(); });
    analyze_map.insert(analyze_btn_slr_table, [this]() { signal_slr_table_analysis(); });

    for (auto it = analyze_map.begin(); it != analyze_map.end(); ++it) {
      QPushButton *btn = it.key();
      std::function<void()> func = it.value();

      connect(btn, &QPushButton::clicked, this, func);
    }
  }

  void setup_signal_view() {
    // 查看按钮组
    view_btn_group = new QButtonGroup(this);
    view_btn_group->addButton(program_btn_view, 0);
    view_btn_group->addButton(lexical_btn_view, 2);
    view_btn_group->addButton(grammar_btn_view, 1);
    view_btn_group->addButton(slr_table_btn_view, 3);

    connect(view_btn_group, &QButtonGroup::buttonClicked, this, [this](QAbstractButton *btn) {
      if (btn == program_btn_view) {
        signal_toggle_file_view(btn, "程序", program_line_edit, program_btn_view);
      } else if (btn == lexical_btn_view) {
        signal_toggle_file_view(btn, "词法", lexical_line_edit, lexical_btn_view);
      } else if (btn == grammar_btn_view) {
        signal_toggle_file_view(btn, "文法", grammar_line_edit, grammar_btn_view);
      } else if (btn == slr_table_btn_view) {
        signal_toggle_file_view(btn, "SLR表", slr_table_line_edit, slr_table_btn_view);
      }
    });
  }

  void setup_signal() {
    setup_signal_browse();
    setup_signal_default();
    setup_signal_analyze();
    setup_signal_view();

    // 结果浏览按钮
    connect(result_btn_browse, &QPushButton::clicked, this, [this]() {
      signal_browse_directory(result_line_edit, "选择结果文件夹");
    });

    // 更多按钮
    connect(more_btn_toggle, &QPushButton::clicked, this, [this]() {
      bool is_visible = bottom_widget->isVisible();
      bottom_widget->setVisible(!is_visible);
      more_btn_toggle->setText(is_visible ? "更多" : "收起");
    });

  }

  void signal_toggle_file_view(QAbstractButton *clicked_btn, const QString &label_prefix, QLineEdit *path_edit, QPushButton *toggle_btn) {
    // 收起右侧视图，重置状态
    view_widget->setVisible(false);

    // 复原所有按钮为“查看”
    for (QAbstractButton *btn : view_btn_group->buttons()) {
      if (btn != clicked_btn) {
        btn->setText("查看");
      }
    }

    // 展开当前按钮
    if (clicked_btn->text() == "查看") {
      QString filename = path_edit->text();

      if (filename.isEmpty()) {
        file_view->setText("未选择文件");
        filename_label->setText(label_prefix);
      } else {
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          QTextStream in(&file);
          QString content = in.readAll();
          file_view->setText(content);

          QFileInfo info(filename);
          filename_label->setText(label_prefix + " " + info.fileName());
        } else {
          file_view->setText("无法打开文件");
          filename_label->setText(label_prefix);
        }
      }

      // 设置可见，更新按钮文字
      filename_label->setVisible(true);
      file_view->setVisible(true);
      view_widget->setVisible(true);
      toggle_btn->setText("隐藏");
    } else {
      // 如果当前是“隐藏”，点击后变为收起
      toggle_btn->setText("查看");
      view_widget->setVisible(false);
    }
  }

  void signal_browse_file(QLineEdit *target_edit, const QString &caption = "选择文件") {
    QString filename = QFileDialog::getOpenFileName(this, caption);
    if (!filename.isEmpty()) {
      target_edit->setText(filename);
    }
  }

  void signal_browse_directory(QLineEdit *target_edit, const QString &caption = "选择文件夹") {
    QString dir = QFileDialog::getExistingDirectory(this, caption);
    if (!dir.isEmpty()) {
      target_edit->setText(dir);
    }
  }

  void signal_lexical_analysis() {
    QString filename = program_line_edit->text();
    if (filename.isEmpty()) {
      QMessageBox::warning(this, "提示", "请先选择程序文件");
      return;
    }

    // TODO: 调用实际词法分析逻辑
    QMessageBox::information(this, "分析", "分析功能未实现");
  }

  void signal_slr_table_analysis() {
    QString filename_grammar = grammar_line_edit->text();
    if (filename_grammar.isEmpty()) {
      QMessageBox::warning(this, "提示", "请先选择文法文件");
      return;
    }
    QString folder_result = result_line_edit->text();
    if (folder_result.isEmpty()) {
      QMessageBox::warning(this, "提示", "请先选择结果文件夹");
      return;
    }
    if (parse_slr_table(filename_grammar, folder_result)) {
      QMessageBox::warning(this, "提示", "SLR表分析成功");
    } else {
      QMessageBox::critical(this, "错误", "SLR表分析失败");
    }
  }
};


int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  CompilerWindow window;
  window.resize(800, 300);
  window.show();

  return app.exec();
}
