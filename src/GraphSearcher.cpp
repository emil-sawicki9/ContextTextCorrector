#include "GraphSearcher.h"

#include <QStack>
#include <QDebug>

GraphSearcher::GraphSearcher()
{

}

NodeVector GraphSearcher::findPathsBetweenTwoNodes(Node *start, Node *end, const int nodeCountBetween, const StringVector& words, const int sentenceId)
{
  QVector<NodeVectorWeighted> paths;
  QStack<Node*> stack;
  stack.push(start);
#ifdef DEBUG
  qDebug() << "SEARCHING PATH BETWEEN" << start->word << end->word << words << "DEPTH =" << nodeCountBetween;
#endif

  currentSentenceId = sentenceId;

  searchGraphDFS(end, nodeCountBetween+1, 0, stack, paths, words);

  return findBestNodeVector(paths);
}

NodeVector GraphSearcher::findPathsBeforeNode(Node* start, const int nodeCountBetween, const StringVector &words)
{
  QVector<NodeVectorWeighted> paths;
  QStack<Node*> stack;
  stack.push(start);
#ifdef DEBUG
  qDebug() << "SEARCHING PATH FROM WORD BACKWARDS" << start->word << words << "DEPTH =" << nodeCountBetween;
#endif

  searchGraphDFSBackward(nodeCountBetween+1, 0, stack, paths, words);

  return findBestNodeVector(paths);
}

NodeVector GraphSearcher::findPathsAfterNode(Node* start, const int nodeCountBetween, const StringVector &words)
{
  QVector<NodeVectorWeighted> paths;
  QStack<Node*> stack;
  stack.push(start);
#ifdef DEBUG
  qDebug() << "SEARCHING PATH FROM WORD FORWARD" << start->word << words << "DEPTH =" << nodeCountBetween;
#endif

  searchGraphDFSForward(nodeCountBetween+1, 0, stack, paths, words);

  return findBestNodeVector(paths);
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
        qDebug() << "appending" << t << " WEIGHT =" << currentWeight;
#endif
        paths.append(qMakePair(stack.toList().toVector(), currentWeight));
      }
      stack.pop();
      return;
    }
    else // search deeper
    {
      const QString& currentWord = words.at(stack.length()-1);
      Q_FOREACH(Edge *e, current->edgesOut)
      {
        Q_FOREACH(const int idx, e->nodeEnd->sentenceIndexes)
        {
          if (currentSentenceId == idx)
          {
            stack.push(e->nodeEnd);
            searchGraphDFS(end, maxDepth, currentWeight + e->value, stack, paths, words);
            break;
          }
        }

        Q_FOREACH(const int idx, e->nodeEnd->sentenceIndexes)
        {
          if (e->nodeEnd->sentenceIndexes.contains(idx) && end->sentenceIndexes.contains(idx) && isSimilarWord(currentWord, e->nodeEnd->word))
          {
            stack.push(e->nodeEnd);
            searchGraphDFS(end, maxDepth, currentWeight + e->value, stack, paths, words);
            break;
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
      Q_FOREACH(Edge *e, current->edgesIn)
      {
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
      Q_FOREACH(Edge *e, current->edgesOut)
      {
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

NodeVector GraphSearcher::findBestNodeVector(const QVector<NodeVectorWeighted> &paths)
{
  NodeVector alternativeNodes;
  int currentEdgeSum = 0;
  Q_FOREACH(NodeVectorWeighted vec, paths)
  {
    // TODO test on nodes with most similar indexes
    if (vec.second >= currentEdgeSum)
      alternativeNodes = vec.first;
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



  QString tmpBase = baseWord;
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
