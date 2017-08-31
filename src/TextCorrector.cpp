#include "TextCorrector.h"

#include <QElapsedTimer>
#include <QDebug>
#include <QStringList>

const StringVector TextCorrector::_sentenceEndings = StringVector() << "." << "!" << "?";
TextCorrector::TextCorrector()
{
}

void TextCorrector::loadLanguage(const QString &fileName, const QString &lang)
{
  _graphParser.loadTextsToGraph(fileName);

  _currentLang = lang;

  currentLanguageChanged();
}

void TextCorrector::changeLanguage(const QString &fileName)
{
  _currentLang = fileName.split("/").last();
  _currentLang.remove(".txt");
  currentLanguageChanged();
}

QString TextCorrector::getCurrentLang() const
{
  return _currentLang;
}

QString TextCorrector::hintSentence(const QString &sentence)
{
  QString sentence_tmp = sentence;
  QStringList words = sentence_tmp.split(' ');

  // trimming and removing empty words
  for (int i = 0 ; i < words.count(); i++)
    words[i] = words.at(i).trimmed();

  Node *perviousNode = _graphParser.getGraph()[words.at(words.count() - 2)];
  if (!perviousNode)
    return "Not found any hint.";

  StringVector wordsToFix;
  wordsToFix.push_back(words.last());
  int edgeLength = 1;
  NodeVector alternativeNodes = _graphSearcher.findPathsAfterNode(perviousNode, edgeLength, wordsToFix);

  bool foundHint = false;
  for (int i = 1 ; i < alternativeNodes.length(); i++)
  {
    if (words[words.length()-1] != alternativeNodes.at(i)->word)
    {
      words[words.length()-1] = "<font color=\"green\">" + alternativeNodes.at(i)->word + "</font>";
      foundHint = true;
    }
  }

  if (!foundHint)
    return "No hint was found!";

  return words.join(" ").trimmed();
}

QString TextCorrector::searchForSentence(const QString &sentence)
{
  QString sentence_tmp = sentence;
  QElapsedTimer timer;
  timer.start();
  // checking for ending mark
  const QString ending = TextCorrector::findSentenceEndingMark(sentence_tmp);
  sentence_tmp.remove(ending);

  QStringList words = sentence_tmp.split(' ');
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

  res = res.trimmed();

  // highlighting words that were corrected
  QString temporarySentence = sentence;
  QStringList first = temporarySentence.replace(".", " .").replace(",", " ,").replace("  ", " ").trimmed().split(" ");
  QStringList corrected = res.split(" ");

  for (int i = 0 ; i < first.length(); i++)
  {
    if (first.at(i) != corrected.at(i))
    {
      corrected[i] = "<font color=\"red\">"+ corrected[i] + "</font>";
    }
  }

  res = corrected.join(" ").replace(" .", ".").replace(" ,", ",").replace("  ", " ").trimmed();
  return res;
}

NodeVector& TextCorrector::fixSentence(NodeVector &vector, const QStringList& sentence)
{
#ifdef DEBUG
  qDebug() << "\nFIXING" << sentence.join(' ');
#endif

  int sentenceIdx = findSentenceIdx(vector);

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

          NodeVector alternativeNodes = _graphSearcher.findPathsBetweenTwoNodes(perviousNode, nextNode, edgeLength, wordsToFix, sentenceIdx);

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

int TextCorrector::findSentenceIdx(NodeVector &vector)
{
  QVector<int> indexes;
  Q_FOREACH(Node* node, vector)
  {
    if (node)
    {
      if (indexes.length() == 0)
      {
        indexes = node->sentenceIndexes;
        continue;
      }
      else
      {
        QVector<int> toRemove;
        Q_FOREACH(const int idx, indexes)
        {
          if (!node->sentenceIndexes.contains(idx))
            toRemove.push_back(idx);
        }

        Q_FOREACH(const int i, toRemove)
        {
          indexes.removeAll(i);
        }
      }
    }
  }

  return indexes.at(0);
}
