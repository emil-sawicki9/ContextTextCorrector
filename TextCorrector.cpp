#include "TextCorrector.h"

#include <QElapsedTimer>
#include <QDebug>

const StringVector TextCorrector::_sentenceEndings = StringVector() << "." << "!" << "?";
TextCorrector::TextCorrector()
{
  _graphParser.loadTextsToGraph();
}

QString TextCorrector::searchForSentence(QString sentence)
{
  QElapsedTimer timer;
  timer.start();
  // checking for ending mark
  const QString ending = TextCorrector::findSentenceEndingMark(sentence);
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

  // searching for all nodes with words from sentence
  bool foundAll = true;
  for (int i = 0 ; i < words.count(); i++)
  {
    if (node)
    {
      if (node->edgesOut.contains(words[i]))
        node = node->edgesOut[words[i]]->nodeEnd;
      else
        node = 0;
    }
    else
      node = _graphParser.getGraph()[words.at(i)];

    if (!node)
      foundAll = false;

    sentenceNodes.push_back(node);
  }

  // adding ending node to end of sentence if not found
  if (!ending.isEmpty() && sentenceNodes.last() == 0)
  {
    Node* endingNode = _graphParser.getGraph()[ending];
    _graphParser.setupConnection(node, endingNode);
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
  qDebug() << ">>> FIXED in " << timer.elapsed() << "ms <<<";
#endif
  return res.trimmed();
}

NodeVector& TextCorrector::fixSentence(NodeVector &vector, const QStringList& sentence)
{
#ifdef DEBUG
  qDebug() << "\nFIXING" << sentence.join(' ');
#endif
  // length between pervious and next node
  int edgeLength = 1;
  for (int i = 0 ; i < vector.count(); i+=edgeLength)
  {
    edgeLength = 1;
    if (vector.at(i) == 0)
    {
      // any next node after
      Node* nextNode = 0;
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

      if (i == 0) // fixing first words
      {
        if (nextNode) // something in sentence is correct
        {
          StringVector wordsToFix;
          for(int k = i+edgeLength -2; k > -1; k--)
            wordsToFix.push_back(sentence.at(k));
          NodeVector alternativeNodes = _graphSearcher.findPathsBeforeNode(nextNode, edgeLength-1, wordsToFix);

          if (alternativeNodes.count() > 0)
          {
            alternativeNodes.pop_front();
            for (int k = 0 ; k < alternativeNodes.length(); k++)
              vector[i+k] = alternativeNodes[alternativeNodes.length() - k - 1];
          }
#ifdef DEBUG
          qDebug() << "fixing" << (edgeLength-1) << "---" << nextNode->word;
#endif
        }
        else // whole sentence is wrong
        {

          // TODO handling whole sentence error
        }
      }
      else // fixing any words between two found
      {
        // node before searched one
        Node* perviousNode = vector.at(i-1);

        if (nextNode) // if only few words are incorrect
        {
#ifdef DEBUG
          qDebug() << "fixing between" << perviousNode->word << "---" << (edgeLength-1) << "---" << nextNode->word;
#endif
          StringVector wordsToFix;
          for(int k = i ; k < i + edgeLength; k++)
            wordsToFix.push_back(sentence.at(k));

          NodeVector alternativeNodes = _graphSearcher.findPathsBetweenTwoNodes(perviousNode, nextNode, edgeLength, wordsToFix);

          if (alternativeNodes.count() > 0)
          {
            alternativeNodes.pop_back();
            alternativeNodes.pop_front();

            for (int k = 0 ; k < alternativeNodes.size() ; k++)
              vector[i + k ] = alternativeNodes[k];
          }
        }
        else // fixing last words
        {
          StringVector wordsToFix;
          for(int k = i ; k < i + edgeLength; k++)
            wordsToFix.push_back(sentence.at(k));
          NodeVector alternativeNodes = _graphSearcher.findPathsAfterNode(perviousNode, edgeLength, wordsToFix);

          if (alternativeNodes.count() > 0)
          {
            alternativeNodes.pop_front();
            for (int k = 0 ; k < alternativeNodes.length(); k++)
              vector[i+k] = alternativeNodes[k];
          }
        }

      }
    }
  }
  return vector;
}

QString TextCorrector::findSentenceEndingMark(const QString &line)
{
  Q_FOREACH(const QString& end, _sentenceEndings)
  {
    if (line.contains(end))
      return end;
  }
  return QString();
}
