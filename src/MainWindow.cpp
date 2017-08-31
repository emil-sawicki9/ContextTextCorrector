#include "MainWindow.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QElapsedTimer>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QFileDialog>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  // Main setup
  this->setCentralWidget(new QWidget(this));

  QVBoxLayout *lay = new QVBoxLayout(this->centralWidget());
  _textEditor = new QTextEdit(this);
  lay->addWidget(_textEditor);
  _resultEditor = new QTextEdit(this);
  _resultEditor->setReadOnly(true);
  QHBoxLayout *butLay = new QHBoxLayout();
  QPushButton * spellingBut = new QPushButton("Correct text", this);
  spellingBut->setMaximumWidth(100);
  connect(spellingBut, &QPushButton::clicked, this, &MainWindow::onCheckSpelling);
  butLay->addWidget(spellingBut, 0, Qt::AlignLeft);
  QPushButton * hintBut = new QPushButton("Hint", this);
  hintBut->setMaximumWidth(100);
  connect(hintBut, &QPushButton::clicked, this, &MainWindow::onHint);
  butLay->addWidget(hintBut, 10, Qt::AlignLeft);

  lay->addLayout(butLay);

  lay->addWidget(_resultEditor);

  connect(_textEditor, &QTextEdit::textChanged, this, &MainWindow::onEditorTextChanged);

  // Menu bar
  QMenu *file = new QMenu("&File",this);
  menuBar()->addMenu(file);
  QAction *loadFile = new QAction("Load file", file);
  connect(loadFile, &QAction::triggered, this, &MainWindow::onLoadFileAction);
  file->addAction(loadFile);
  QAction *exit = new QAction("Exit", file);
  file->addAction(exit);
  connect(exit, &QAction::triggered, this, &MainWindow::close);

  QMenu *lang = new QMenu("&Language", this);
  menuBar()->addMenu(lang);
  QAction *loadLang = new QAction("Change language", lang);
  connect(loadLang, &QAction::triggered, this, &MainWindow::onChangeLanguageAction);
  lang->addAction(loadLang);

  // status bar
  _currLangLabel = new QLabel(this);
  this->statusBar()->addWidget(_currLangLabel);
  connect(&_corrector, &TextCorrector::currentLanguageChanged, this, &MainWindow::onCurrentLangChanged);

  QString engLangFile("texts/eng.txt");
  if (QFile::exists(engLangFile))
  {
    _corrector.loadLanguage(engLangFile, "eng");
  }
  else
    onChangeLanguageAction();

  _textEditor->setPlainText(QString("There were doors all round the hall"));
//  QTimer::singleShot(0, this, &MainWindow::close);
}

void MainWindow::onHint()
{
  QString lastSentence = _textEditor->toPlainText().split(".").last();
  lastSentence = lastSentence.trimmed();
  if (lastSentence.split(" ").length() < 2)
    _resultEditor->setText("Not enough data to hint word");
  else
    _resultEditor->setText(_corrector.hintSentence(lastSentence));
}

void MainWindow::onLoadFileAction()
{
  QString fileName = QFileDialog::getOpenFileName(0, "Language file", QString(), "*.txt");
  QFile::copy(fileName, "texts/"+fileName.split("/").last());
  if (!fileName.isEmpty())
    _corrector.changeLanguage(fileName);
}

void MainWindow::onChangeLanguageAction()
{
  QDir dir("texts");
  QStringList files = dir.entryList(QStringList() << "*.txt");
  if (files.isEmpty())
  {
    QMessageBox::warning(0, "Warning", "Texts directory is empty!");
  }
  else
  {
    QStringList languages;
    for (int i = 0 ; i < files.length();i++)
    {
      QString path = files.at(i);
      languages << path.remove(".txt");
      files[i] = "texts/" + files[i];
    }

    QDialog dlg;
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    QLabel *label = new QLabel(tr("Select dictionary language"),&dlg);
    lay->addWidget(label);
    QListWidget *list = new QListWidget(&dlg);
    lay->addWidget(list);
    list->addItems(languages);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    lay->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::close);
    connect(buttonBox, &QDialogButtonBox::rejected, list, &QListWidget::clear);
    connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::close);

    dlg.exec();

    if (list->count() && list->currentRow() >= 0)
      _corrector.changeLanguage(files.at(list->currentRow()));
  }
}

void MainWindow::onCurrentLangChanged()
{
  _currLangLabel->setText("Current language: <b>"+_corrector.getCurrentLang()+"</b>");
}

void MainWindow::onCheckSpelling()
{
  _resultEditor->setText(_corrector.searchForSentence(_textEditor->toPlainText()));
}

void MainWindow::onEditorTextChanged()
{
//  qDebug() << _textEditor->toPlainText();
}
