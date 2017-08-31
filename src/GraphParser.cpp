#include "GraphParser.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QStack>
#include <QElapsedTimer>

#include "TextCorrector.h"

GraphParser::GraphParser()
  : _sentenceIndexCounter(0)
{
  _graph = Graph();
  debugEdgeCounter = 0;
}

const Graph& GraphParser::getGraph() const
{
  return _graph;
}

void GraphParser::loadTextsToGraph(const QString &fileName)
{
  // TODO deleting all pointers in graph
  _graph.clear();
  qDebug() << "LOADING DATA";
  QElapsedTimer timer;
  timer.start();
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
      QString ending = TextCorrector::findSentenceEndingMark(line);
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
          QString subEnding = TextCorrector::findSentenceEndingMark(subsentence);
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
    qDebug() << "CANNOT OPEN FILE" << (QDir::currentPath() + "/"+file.fileName()) << file.errorString();

  qDebug() << "loaded" << _graph.count() << "words and" << debugEdgeCounter << "edges in" << timer.elapsed() << "ms.";
}

void GraphParser::parseToGraph(QString &sentence)
{
  // removing parentheses and Capitalizing first letter
  sentence.remove("(").remove(")");
  sentence = sentence.left(1).toUpper()+sentence.mid(1);
  // checking if sentence start from letter or is long enough
  if (!sentence.at(0).isLetter() || sentence.split(" ").count() < 3)
    return;

  // finding ending of sentence
  const QString endMark = TextCorrector::findSentenceEndingMark(sentence);
  const int sentenceIdx = _sentenceIndexCounter++;
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

    perviousNode = setupConnection(perviousNode, word, sentenceIdx);
  }
}

Node* GraphParser::setupConnection(Node *nodeLeft, Node *nodeRight)
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
    if (nodeLeft->edgesOut.count() > debugEdgeCounter)
    {
        debugEdgeMaxCounter = nodeLeft->edgesOut.count();
        debugMostEdgeWord = nodeLeft->word;
    }
  }
  return nodeRight;
}

Node* GraphParser::setupConnection(Node *nodeLeft, const QString& nodeRightWord, const int sentenceIdx)
{
  // finding or creating node to connect to
  Node* nodeRight = _graph[nodeRightWord];

  if (!nodeRight)
  {
    nodeRight = new Node;
    nodeRight->word = nodeRightWord;
    nodeRight->sentenceIndexes.push_back(sentenceIdx);
    _graph.insert(nodeRightWord, nodeRight);
  }
  else if (!nodeRight->sentenceIndexes.contains(sentenceIdx))
  {
    nodeRight->sentenceIndexes.push_back(sentenceIdx);
  }

  return setupConnection(nodeLeft, nodeRight);
}
