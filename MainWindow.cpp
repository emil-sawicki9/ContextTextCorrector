#include "MainWindow.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QStack>

#define DEBUG

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

  loadTextsToGraph("./text.txt");

  qDebug() <<"RESULT" << searchForSentence(QString("The weather was qiute cool and cloudy and it turned out a showery afternoon."));

  QTimer::singleShot(0, this, &MainWindow::close);
}

MainWindow::~MainWindow()
{

}

QString MainWindow::searchForSentence(QString sentence)
{
  // checking for ending mark
  const QString ending = findSentenceEndingMark(sentence);
  sentence.remove(ending);

  QStringList words = sentence.split(' ');
  if (!ending.isEmpty())
    words.append(ending);

  // trimming and removing empty words
  for (int i = 0 ; i < words.count(); i++)
    words[i] = words.at(i).trimmed();

  NodeVector sentenceNodes;
  // getting first node
  Node* node = 0;
  if (words.count() > 0)
    node = _graph[words.at(0)];
  sentenceNodes.push_back(node);

  // searching for all nodes with words from sentence
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
    else
      node = _graph[words.at(i)];
    sentenceNodes.push_back(node);
  }

  // adding ending node to end of sentence if not found
  if (!ending.isEmpty() && sentenceNodes.last() == 0)
  {
    Node* endingNode = _graph[ending];
    setupConnection(node, endingNode);
    sentenceNodes[sentenceNodes.length() - 1] = endingNode;
  }

  if (!foundAll)
    fixSentence(sentenceNodes, words);
#ifdef DEBUG
  QString res = QString();
  Q_FOREACH(Node* n, sentenceNodes)
  {
    if (n)
      res += n->word + " ";
    else
      res += "NULL ";
  }
#endif

  return res.trimmed();
}

NodeVector& MainWindow::fixSentence(NodeVector &vector, const QStringList& sentence)
{
#ifdef DEBUG
  qDebug() << "FIXING" << sentence.join(' ');
#endif
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
#ifdef DEBUG
          qDebug() << "fixing between" << perviousNode->word << "---" << (edgeLength-1) << "---" << nextNode->word;
#endif
          QVector<NodeVector> paths = findPathsBetweenTwoNodes(perviousNode, nextNode, edgeLength-1);
          NodeVector alternativeNodes;
          Q_FOREACH(NodeVector vec, paths)
          {
            vec.pop_back();
            vec.pop_front();
            alternativeNodes.push_back(vec.first());
          }

          vector[i] = findNodeWithSimilarWord(currentWord, alternativeNodes);
          i += edgeLength-1;
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

QVector<NodeVector> MainWindow::findPathsBetweenTwoNodes(Node *start, Node *end, const int nodeCountBetween)
{
  QVector<NodeVector> paths;
  QStack<Node*> stack;
  stack.push(start);
#ifdef DEBUG
  qDebug() << "SEARCHING PATH" << start->word << nodeCountBetween << end->word;
#endif

  searchGraphDFS(end, nodeCountBetween+2, stack, paths);

  return paths;
}

void MainWindow::searchGraphDFS(Node *end, const int maxDepth, QStack<Node*>& stack, QVector<NodeVector>& paths)
{
  while(!stack.isEmpty())
  {
    Node* current = stack.top();
    if (stack.length() == maxDepth)
    {
      if (current == end) // founding correct path
      {
#ifdef DEBUG
//        QString t;
//        Q_FOREACH(Node* n, stack)
//          t += n->word + " ";
//        qDebug() << "appending" << t;
#endif
        paths.append(stack.toList().toVector());
      }
      stack.pop();
      return;
    }
    else // search deeper
    {
      Q_FOREACH(Edge *e, current->edgesOut)
      {
        stack.push(e->nodeEnd);
        searchGraphDFS(end, maxDepth, stack, paths);
      }
      stack.pop();
      return;
    }
  }
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

  if (vec.length() > 0)
    return vec.at(0);
  else
    return 0;
}

void MainWindow::onEditorTextChanged()
{
  qDebug() << _textEditor->toPlainText();
}

QString MainWindow::findSentenceEndingMark(const QString &line)
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
  // removing parentheses and Capitalizing first letter
  sentence.remove("(").remove(")");
  sentence = sentence.left(1).toUpper()+sentence.mid(1);
  // checking if sentence start from letter or is long enough
  if (!sentence.at(0).isLetter() || sentence.split(" ").count() < 3)
    return;

  // finding ending of sentence
  const QString endMark = findSentenceEndingMark(sentence);
  sentence = sentence.remove("!").remove("?").remove(".");
  QStringList words = sentence.split(' ');

  if (words.isEmpty()|| endMark.isEmpty())
    return;

  // appending ending mark to list of words
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

Node* MainWindow::setupConnection(Node *nodeLeft, Node *nodeRight)
{
  if (nodeLeft && nodeRight)
  {
    // finding or creating edge
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

Node* MainWindow::setupConnection(Node *nodeLeft, const QString& nodeRightWord)
{
  // finding or creating node to connect to
  Node* nodeRight = _graph[nodeRightWord];

  if (!nodeRight)
  {
    nodeRight = new Node;
    nodeRight->word = nodeRightWord;
    _graph.insert(nodeRightWord, nodeRight);
  }

  return setupConnection(nodeLeft, nodeRight);
}

void MainWindow::loadTextsToGraph(const QString& fileName)
{
  // opening file
  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly))
  {
    QTextStream in(&file);
    QString sentence = QString();
    while (!in.atEnd())
    {
      // reading line and removing special signs
      QString line = in.readLine().trimmed();
      if (line.startsWith("#") || line.startsWith("*") || line.contains("http:") || line.contains("-") || line.contains("@"))
        continue;

      line.remove("‘").remove("\"").remove("’").replace(";", ".").remove(",");

      // finding sentence ending mark
      QString ending = findSentenceEndingMark(line);
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
          QString subEnding = findSentenceEndingMark(subsentence);
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
        // if line have no ending and has new line after it
        // (it's probably some book notes then)
        sentence = QString();
      }
      else
      {
        // appending new line to sentence
        sentence += line + " ";
      }
    }
  }
  else
    qDebug() << "CANNOT OPEN FILE" << file.errorString();

  qDebug() << "loaded" << _graph.count() << "words and" << debugEdgeCounter << "edges";
}
