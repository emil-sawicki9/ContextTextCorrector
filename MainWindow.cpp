#include "MainWindow.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , _sentenceEndings(QVector<QString>() << "." << "!" << "?")
{
  debugCounter = 0;
  this->setCentralWidget(new QWidget(this));

  QVBoxLayout *lay = new QVBoxLayout(this->centralWidget());
  _textEditor = new QTextEdit(this);
  _resultEditor = new QTextEdit(this);
  _resultEditor->setReadOnly(true);

  lay->addWidget(_textEditor);
  lay->addWidget(_resultEditor);

  connect(_textEditor, &QTextEdit::textChanged, this, &MainWindow::onEditorTextChanged);

  loadTextsToGraph();
}

MainWindow::~MainWindow()
{

}

void MainWindow::onEditorTextChanged()
{
  qDebug() << _textEditor->toPlainText();
}

QString MainWindow::findSentenceEnding(const QString &line)
{
  Q_FOREACH(const QString& end, _sentenceEndings)
  {
    if (line.contains(end))
      return end;
  }
  return QString();
}

void MainWindow::parseToGraph(QString &sentence)
{
  sentence.remove("(");
  sentence.remove(")");
  sentence = sentence.left(1).toUpper()+sentence.mid(1);
  if (!sentence.at(0).isLetter() || sentence.split(" ").count() < 3)
    return;

  const QString endMark = findSentenceEnding(sentence);
  sentence = sentence.remove("!").remove("?");
  QStringList words = sentence.split(' ');

  if (words.isEmpty()|| endMark.isEmpty())
    return;

  if (QString(words.last()).trimmed() != endMark)
    words.append(endMark);

  Q_FOREACH(QString w, words)
  {
    if (w.trimmed().isEmpty())
      words.removeAll(w);
  }

  Node* perviousNode = 0;
  const QString comma = QString(",");
  Q_FOREACH(QString word, words)
  {
    if (word.startsWith(comma))
    {
      word.remove(0,1);
      perviousNode = setupConnection(perviousNode, comma);
    }
    else if (word.endsWith(comma))
    {
      word.chop(1);
      perviousNode = setupConnection(perviousNode, comma);
    }

    perviousNode = setupConnection(perviousNode, word);
  }
}

Node* MainWindow::setupConnection(Node *nodeLeft, const QString& nodeRightWord)
{
  Node* nodeRight = 0;
  if (_graph.contains(nodeRightWord))
  {
    nodeRight = _graph[nodeRightWord];
  }
  else
  {
    nodeRight = new Node;
    nodeRight->word = nodeRightWord;
    _graph.insert(nodeRightWord, nodeRight);
  }

  if (nodeLeft && nodeRight)
  {
    Edge * connection = 0;
    // if node before NOT have connetion to current node
    if (!nodeLeft->edgesTo.contains(nodeRight->word))
    {
      connection = new Edge;
      debugCounter++;
      connection->nodeFrom = nodeLeft;
      connection->nodeTo = nodeRight;
      nodeLeft->edgesTo.insert(nodeRight->word, connection);
    }
    else
    {
      // popup connection value change
    }

    // if current node before NOT have connetion to pervious node
    if (!nodeRight->edgesFrom.contains(nodeLeft->word))
    {
      nodeRight->edgesFrom.insert(nodeLeft->word, connection);
    }
  }

  return nodeRight;
}

void MainWindow::loadTextsToGraph()
{
  QFile file("./text.txt");
  if (file.open(QIODevice::ReadOnly))
  {
    QTextStream in(&file);
    QString sentence = QString();
    while (!in.atEnd())
    {
      QString line = in.readLine().trimmed();
      if (line.startsWith("#") || line.startsWith("*") || line.contains("http:") || line.contains("-") || line.contains("@"))
        continue;

      line.remove("‘");
      line.remove("\"");
      line.remove("’");
      line.replace(";", ".");

      QString ending = findSentenceEnding(line);
      if (!ending.isEmpty())
      {
        QStringList sentences = line.split(ending);
        sentence += sentences.at(0) + ending;

        sentence = sentence.trimmed();
        int idxParLeft = sentence.indexOf("(");
        int idxParRight = sentence.indexOf(")");
        if (idxParLeft != -1 && idxParRight != 1)
        {
          QString subsentence = sentence.mid(idxParLeft+1, idxParRight - idxParLeft-1);
          QString subEnding = findSentenceEnding(subsentence);
          if (subEnding.isEmpty())
            subsentence.append(".");

          parseToGraph(subsentence);

          sentence.remove(idxParLeft, idxParRight - idxParLeft +1);
        }

        parseToGraph(sentence);

        sentence = sentences.at(1);
      }
      else if (line.isEmpty() && !sentence.isEmpty())
      {
        sentence = QString();
      }
      else
      {
        sentence += line + " ";
      }
    }
  }
  else
    qDebug() << "CANNOT OPEN FILE" << file.errorString();

  qDebug() << "loaded" << _graph.count() << "words and " << debugCounter << "edges";
}
