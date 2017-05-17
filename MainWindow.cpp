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
  debugEdgeCounter = 0;
  this->setCentralWidget(new QWidget(this));

  QVBoxLayout *lay = new QVBoxLayout(this->centralWidget());
  _textEditor = new QTextEdit(this);
  _resultEditor = new QTextEdit(this);
  _resultEditor->setReadOnly(true);

  lay->addWidget(_textEditor);
  lay->addWidget(_resultEditor);

  connect(_textEditor, &QTextEdit::textChanged, this, &MainWindow::onEditorTextChanged);

  loadTextsToGraph();

  qDebug() <<"RESULT" << searchForSentence(QString("The weather wsa qute cool and cloudy and it turned out a shoery afternoon."));

  QTimer::singleShot(0, this, &MainWindow::close);
}

MainWindow::~MainWindow()
{

}

QString MainWindow::searchForSentence(QString sentence)
{
  // checking for ending mark
  QString ending = findSentenceEnding(sentence);
  if (!sentence.isEmpty())
    sentence.remove(ending);

  QStringList words = sentence.split(' ');
  if (!ending.isEmpty())
    words.append(ending);

  // trimming and removing empty words
  for (int i = 0 ; i < words.count(); i++)
  {
    words[i] = words.at(i).trimmed();
  }

  NodeVector sentenceNodes;

  // getting first node
  Node* node = 0;
  if (words.count() > 0 && _graph.contains(words.at(0)))
  {
    node = _graph[words.at(0)];
  }
  sentenceNodes.push_back(node);

  // searching for all nodes
  bool foundAll = true;
  for (int i = 1 ; i < words.count(); i++)
  {
    if (node)
    {
      if (node->edgesOut.contains(words[i]))
        node = node->edgesOut[words[i]]->nodeEnd;
      else
      {
        node = 0;
        foundAll = false;
      }
    }
    else if (_graph.contains(words.at(i)))
    {
      node = _graph[words.at(i)];
    }
    sentenceNodes.push_back(node);
  }

  // adding ending node to end of sentence if not found
  if (!ending.isEmpty() && sentenceNodes.last() == 0)
  {
    Node* endingNode = _graph[ending];
    // adding connection between last word and ending
//    if (node && endingNode)
//    {
//      Edge *conn = new Edge;
//      conn->value = 1;
//      conn->nodeStart = node;
//      conn->nodeEnd = endingNode;
//      node->edgesOut.insert(ending ,conn);
//      endingNode->edgesIn.insert(node->word, conn);
//    }
    sentenceNodes[sentenceNodes.length() - 1] = endingNode;
  }

  if (!foundAll)
  {
    fixSentence(sentenceNodes, words);
  }

  QString res = QString();
  Q_FOREACH(Node* n, sentenceNodes)
  {
    if (n)
      res += n->word + " ";
    else
      res += "NULL ";
  }

  return res.trimmed();
}

NodeVector& MainWindow::fixSentence(NodeVector &vector, const QStringList& sentence)
{
  qDebug() << "FIXING" << sentence.join(' ');
  for (int i = 0 ; i < vector.count(); i++)
  {
    if (vector.at(i) == 0)
    {
      const QString& currentWord = sentence.at(i);

      if (i == 0) // fixing first word
      {

      }
      else // fixing any other word
      {
        // node before searched one
        Node* perviousNode = vector.at(i-1);
        // any next node after
        Node* nextNode = 0;
        // length between pervious and next node
        int edgeLength = 1;
        // searching for next node
        for (int j = i+1 ; j < vector.count(); j++)
        {
          edgeLength++;
          if (vector.at(j))
          {
            nextNode = vector.at(j);
            break;
          }
        }

        if (nextNode) // if only few words are incorrect
        {
          qDebug() << perviousNode->word << "---" << (edgeLength-1) << "---" << nextNode->word;
          // one node is wrong
          if (edgeLength == 2)
          {
            Edge *bestEdge = 0;
            NodeVector alternativeNodes;
            Q_FOREACH(const QString& word, perviousNode->edgesOut.keys())
            {
              if (nextNode->edgesIn.contains(word))
              {
                alternativeNodes.push_back(nextNode->edgesIn[word]->nodeStart);
                Edge *e = nextNode->edgesIn[word];
                Edge *pe = perviousNode->edgesOut[word];
                if (bestEdge)
                {
                  if (e->value > bestEdge->value)
                    bestEdge = e;
                  if (pe->value > bestEdge->value)
                    bestEdge = pe;
                }
                else
                  bestEdge = e->value > pe->value ? e : pe;
              }
            }


            Node* node = findNodeWithSimilarWord(currentWord, alternativeNodes);
            if (node)
              vector[i] = node;
            else if (bestEdge)
              vector[i] = bestEdge->nodeStart == perviousNode ? bestEdge->nodeEnd : bestEdge->nodeStart;
          }
          // multiple nodes are wrong
          else
          {
            QVector<NodeVector> paths;
            i += edgeLength-1;
          }
        }
        else // if all words are incorrect
        {
          return vector;
        }

      }
    }
  }
  return vector;
}

Node* MainWindow::findNodeWithSimilarWord(const QString &word, NodeVector &vec)
{
  QString sortedWord = word;
  std::sort(sortedWord.begin(), sortedWord.end());

  const int start = (int)'a';
  const int end = (int)'z';

  Q_FOREACH(Node* node, vec)
  {
    QString nodeWord = node->word;
    std::sort(nodeWord.begin(), nodeWord.end());
    // when words have same length
    if (nodeWord.length() == sortedWord.length())
    {
      if (nodeWord == sortedWord)
        return node;
    }
    // when word is bigger than node word
    else if (nodeWord.length() > sortedWord.length())
    {
      for (int i = start ; i <= end; i++)
      {
        QString tmpWord = sortedWord + (char)i;
        std::sort(tmpWord.begin(), tmpWord.end());
        if (tmpWord == nodeWord)
        {
          return node;
        }
      }
    }
    // when word is smaller than node word
    else
    {
      for (int i = start ; i <= end; i++)
      {
        QString tmpWord = nodeWord + (char)i;
        std::sort(tmpWord.begin(), tmpWord.end());
        if (tmpWord == sortedWord)
        {
          return node;
        }
      }
    }

  }

  return 0;
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
  sentence = sentence.remove("!").remove("?").remove(".");
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
//  const QString comma = QString(",");
  Q_FOREACH(QString word, words)
  {
    // checking for comma
//    if (word.startsWith(comma))
//    {
//      word.remove(0,1);
//      perviousNode = setupConnection(perviousNode, comma);
//    }
//    else if (word.endsWith(comma))
//    {
//      word.chop(1);
//      perviousNode = setupConnection(perviousNode, comma);
//    }

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
    if (!nodeLeft->edgesOut.contains(nodeRight->word))
    {
      connection = new Edge;
      debugEdgeCounter++;
      connection->value = 1;
      connection->nodeStart = nodeLeft;
      connection->nodeEnd = nodeRight;
      nodeLeft->edgesOut.insert(nodeRight->word, connection);
    }
    else
    {
      // popup connection value change
      nodeLeft->edgesOut[nodeRight->word]->value++;
    }

    // if current node before NOT have connetion to pervious node
    if (!nodeRight->edgesIn.contains(nodeLeft->word))
    {
      nodeRight->edgesIn.insert(nodeLeft->word, connection);
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

      line.remove("‘").remove("\"").remove("’").replace(";", ".").remove(",");

      QString ending = findSentenceEnding(line);
      if (!ending.isEmpty())
      {
        QStringList sentences = line.split(ending);
        sentence += sentences.at(0) + ending;

        sentence = sentence.trimmed();

        // getting sentence out of parentheses
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

        // parsing sentence
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

  qDebug() << "loaded" << _graph.count() << "words and" << debugEdgeCounter << "edges";
}
