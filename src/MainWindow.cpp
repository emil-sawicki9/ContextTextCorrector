#include "MainWindow.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  this->setCentralWidget(new QWidget(this));

  QVBoxLayout *lay = new QVBoxLayout(this->centralWidget());
  _textEditor = new QTextEdit(this);
  _resultEditor = new QTextEdit(this);
  _resultEditor->setReadOnly(true);
  QPushButton * spellingBut = new QPushButton("GO", this);
  connect(spellingBut, &QPushButton::clicked, this, &MainWindow::onCheckSpelling);

  lay->addWidget(_textEditor);
  lay->addWidget(spellingBut);
  lay->addWidget(_resultEditor);

  connect(_textEditor, &QTextEdit::textChanged, this, &MainWindow::onEditorTextChanged);

  _textEditor->setPlainText(QString("There were doors all round the hall"));

//  QTimer::singleShot(0, this, &MainWindow::close);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onCheckSpelling()
{
  _resultEditor->setPlainText(_corrector.searchForSentence(_textEditor->toPlainText()));
}

void MainWindow::onEditorTextChanged()
{
//  qDebug() << _textEditor->toPlainText();
}
