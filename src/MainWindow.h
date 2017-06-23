#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "TextCorrector.h"


class QTextEdit;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void onEditorTextChanged();
  void onCheckSpelling();

private:
  QTextEdit *_textEditor, *_resultEditor;
  TextCorrector _corrector;
};

#endif // MAINWINDOW_H
