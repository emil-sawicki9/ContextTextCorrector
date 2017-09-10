#include "GraphSearcher.h"

#include <QStack>
#include <QDebug>

bool compareNodeVector(const NodeVectorWeighted& left, const NodeVectorWeighted& right)
{
  return left.second > right.second;
}


GraphSearcher::GraphSearcher()
{

}

QVector<NodeVector> GraphSearcher::findPathsBetweenTwoNodes(Node *start, Node *end, const int nodeCountBetween, const StringVector& words, const QVector<int>& sentenceId)
{
  QVector<NodeVectorWeighted> paths;
  QStack<Node*> stack;
  stack.push(start);
#ifdef DEBUG
  qDebug() << "SEARCHING PATH BETWEEN" << start->word << end->word << words << "DEPTH =" << nodeCountBetween;
#endif

  currentSentenceId = sentenceId;

  searchGraphDFS(end, nodeCountBetween+1, 0, stack, paths, words);

  return findBestNodeVectors(paths);
}

QVector<NodeVector> GraphSearcher::findPathsBeforeNode(Node* start, const int nodeCountBetween, const StringVector &words)
{
  QVector<NodeVectorWeighted> paths;
  QStack<Node*> stack;
  stack.push(start);
#ifdef DEBUG
  qDebug() << "SEARCHING PATH FROM WORD BACKWARDS" << start->word << words << "DEPTH =" << nodeCountBetween;
#endif

  searchGraphDFSBackward(nodeCountBetween+1, 0, stack, paths, words);

  return findBestNodeVectors(paths);
}

QVector<NodeVector> GraphSearcher::findPathsAfterNode(Node* start, const int nodeCountBetween, const StringVector &words)
{
  QVector<NodeVectorWeighted> paths;
  QStack<Node*> stack;
  stack.push(start);
#ifdef DEBUG
  qDebug() << "SEARCHING PATH FROM WORD FORWARD" << start->word << words << "DEPTH =" << nodeCountBetween;
#endif

  searchGraphDFSForward(nodeCountBetween+1, 0, stack, paths, words);

  return findBestNodeVectors(paths);
}

void GraphSearcher::searchGraphDFS(Node *end, const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words)
{
  while(!stack.isEmpty())
  {
    Node* current = stack.top();
    if (stack.length() == maxDepth)
    {
      if (current == end) // founding correct path
      {
#ifdef DEBUG
        QString t;
        Q_FOREACH(Node* n, stack)
          t += n->word + " ";
        qDebug() << Q_FUNC_INFO << "appending" << t << " WEIGHT =" << currentWeight;
#endif
        paths.append(qMakePair(stack.toList().toVector(), currentWeight));
      }
      stack.pop();
      return;
    }
    else // search deeper
    {
      const QString& currentWord = words.at(stack.length()-1);
      bool foundByIndex = false;
      float count = 0;
      Q_FOREACH(Edge *e, current->edgesOut)
      {
        if (stack.count() == 1)
        {
          count += 0.5;
          emit valueChanged(std::floor(count));
        }
        Q_FOREACH(const int idx, e->nodeEnd->sentenceIndexes)
        {
          if (currentSentenceId.contains(idx))
          {
            stack.push(e->nodeEnd);
            searchGraphDFS(end, maxDepth, currentWeight + e->value, stack, paths, words);
            foundByIndex = true;
            break;
          }
        }
      }

      if (!foundByIndex)
      {
        Q_FOREACH(Edge *e, current->edgesOut)
        {
          if (stack.count() == 1)
          {
            count += 0.5;
            emit valueChanged(std::floor(count));
          }
          Q_FOREACH(const int idx, e->nodeEnd->sentenceIndexes)
          {
            if (e->nodeEnd->sentenceIndexes.contains(idx) && end->sentenceIndexes.contains(idx) && isSimilarWord(currentWord, e->nodeEnd->word))
            {
              qDebug() << "by_similar" << idx << e->nodeEnd->word;
              stack.push(e->nodeEnd);
              searchGraphDFS(end, maxDepth, currentWeight + e->value, stack, paths, words);
              break;
            }
          }
        }
      }
      stack.pop();
      return;
    }
  }
}

void GraphSearcher::searchGraphDFSBackward(const int maxDepth, const int currentWeight, QStack<Node *> &stack, QVector<NodeVectorWeighted> &paths, const StringVector &words)
{
  while(!stack.isEmpty())
  {
    Node* current = stack.top();
    if (stack.length() == maxDepth)
    {
#ifdef DEBUG
        QString t;
        Q_FOREACH(Node* n, stack)
          t += n->word + " ";
        qDebug() << "appending" << t << " WEIGHT =" << currentWeight;
#endif
      paths.append(qMakePair(stack.toList().toVector(), currentWeight));
      stack.pop();
      return;
    }
    else // search deeper
    {
      const QString& currentWord = words.at(stack.length()-1);
      int count = 0;
      Q_FOREACH(Edge *e, current->edgesIn)
      {
        if (stack.count() == 1)
        {
          emit valueChanged(++count);
        }
        Q_FOREACH(const int idx, e->nodeStart->sentenceIndexes)
        {
          if (current->sentenceIndexes.contains(idx))
          {
            if (isSimilarWord(currentWord, e->nodeStart->word))
            {
              stack.push(e->nodeStart);
              searchGraphDFSBackward(maxDepth, currentWeight + e->value, stack, paths, words);
              break;
            }
//            if (currentWord == e->nodeStart->word)
//              break;
          }
        }
      }
      stack.pop();
      return;
    }
  }
}

void GraphSearcher::searchGraphDFSForward(const int maxDepth, const int currentWeight, QStack<Node *> &stack, QVector<NodeVectorWeighted> &paths, const StringVector &words)
{
  while(!stack.isEmpty())
  {
    Node* current = stack.top();
    if (stack.length() == maxDepth)
    {
#ifdef DEBUG
        QString t;
        Q_FOREACH(Node* n, stack)
          t += n->word + " ";
        qDebug() << "appending" << t << " WEIGHT =" << currentWeight;
#endif
      paths.append(qMakePair(stack.toList().toVector(), currentWeight));
      stack.pop();
      return;
    }
    else // search deeper
    {
      const QString& currentWord = words.at(stack.length()-1);
      int count = 0;
      Q_FOREACH(Edge *e, current->edgesOut)
      {
        if (stack.count() == 1)
        {
          emit valueChanged(++count);
        }
        Q_FOREACH(const int idx, e->nodeEnd->sentenceIndexes)
        {
          if (current->sentenceIndexes.contains(idx))
          {
            if (isSimilarWord(currentWord, e->nodeEnd->word))
            {
              stack.push(e->nodeEnd);
              searchGraphDFSForward(maxDepth, currentWeight + e->value, stack, paths, words);
              break;
            }
//        if (currentWord == e->nodeEnd->word)
//          break;
          }
        }
      }
      stack.pop();
      return;
    }
  }
}

QVector<NodeVector> GraphSearcher::findBestNodeVectors(QVector<NodeVectorWeighted> &paths)
{
  QVector<NodeVector> alternativeNodes;

  std::sort( paths.begin(), paths.end(), compareNodeVector );

  for (int i = 0 ; i < paths.length() && i < 10; i++)
  {
    alternativeNodes.push_back(paths.at(i).first);
  }

  return alternativeNodes;
}

bool GraphSearcher::isSimilarWord(QString left, QString right)
{
  const int start = (int)'a';
  const int end = (int)'z';
  std::sort(left.begin(), left.end());
  std::sort(right.begin(), right.end());
  QString searchedWord, baseWord;
  // when words have same length
  if (left.length() == right.length())
  {
        if (left == right)
            return true;
  }
  // when word is bigger than node word
  else if (left.length() > right.length())
  {
      baseWord = right;
      searchedWord = left;
  }
  // when word is smaller than node word
  else
  {
      baseWord = left;
      searchedWord = right;
  }

  for (int l = 0 ; l < searchedWord.length() - baseWord.length(); l++)
  {
      for (int i = start ; i <= end; i++)
      {
        QString tmpWord = baseWord + (char)i;
        std::sort(tmpWord.begin(), tmpWord.end());
        if (tmpWord == searchedWord)
        {
          return true;
        }
      }
  }

  return false;
}

void GraphSearcher::findSimilarWord(const int length, const QString &word, const QString &searchedWord)
{
  Q_UNUSED(length);
  Q_UNUSED(word);
  Q_UNUSED(searchedWord);
}

Node* GraphSearcher::findNodeWithSimilarWord(const QString &word, NodeVector &vec)
{
  QString sortedWord = word;
  std::sort(sortedWord.begin(), sortedWord.end());

  Q_FOREACH(Node* node, vec)
  {
    if (isSimilarWord(sortedWord, node->word))
      return node;
  }

  if (vec.length() > 0)
    return vec.at(0);
  else
    return 0;
}
