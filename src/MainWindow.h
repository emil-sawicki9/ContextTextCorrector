#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>

#include "TextCorrector.h"


class QTextEdit;
class QLabel;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  static MainWindow* instance();
  void init();

public slots:
  void updateResult(const QString &result);

private slots:
  void onEditorTextChanged();
  void onCheckSpelling();
  void onHint();
  void onLoadFileAction();
  void onChangeLanguageAction();
  void onCurrentLangChanged();

  void loadLanguageOnStart();

private:
  MainWindow(QWidget *parent = 0);
  static MainWindow* _instance;
  QTextEdit *_textEditor, *_resultEditor;
  TextCorrector *_corrector;
  QLabel *_currLangLabel;
};

#endif // MAINWINDOW_H
