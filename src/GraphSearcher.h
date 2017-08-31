#ifndef GRAPHSEARCHER_H
#define GRAPHSEARCHER_H

#include "GraphElements.h"

class GraphSearcher
{
public:
  GraphSearcher();

  NodeVector findPathsBetweenTwoNodes(Node* start, Node* end, const int nodeCountBetween, const StringVector& words, const int sentenceId);
  NodeVector findPathsBeforeNode(Node* start, const int nodeCountBetween, const StringVector& words = StringVector());
  NodeVector findPathsAfterNode(Node* start, const int nodeCountBetween, const StringVector& words = StringVector());

  void searchGraphDFS(Node* end, const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words);
  void searchGraphDFSBackward(const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words);
  void searchGraphDFSForward(const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words);

  NodeVector findBestNodeVector(const QVector<NodeVectorWeighted> &paths);

  bool isSimilarWord(QString left, QString right);

  void findSimilarWord(const int length, const QString &word, const QString &searchedWord);

  Node* findNodeWithSimilarWord(const QString& word, NodeVector& vec);

private:
  int currentSentenceId;

};

#endif // GRAPHSEARCHER_H
