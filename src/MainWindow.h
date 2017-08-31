#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "TextCorrector.h"


class QTextEdit;
class QLabel;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);

private slots:
  void onEditorTextChanged();
  void onCheckSpelling();
  void onHint();
  void onLoadFileAction();
  void onChangeLanguageAction();
  void onCurrentLangChanged();

private:
  QTextEdit *_textEditor, *_resultEditor;
  TextCorrector _corrector;
  QLabel *_currLangLabel;
};

#endif // MAINWINDOW_H
