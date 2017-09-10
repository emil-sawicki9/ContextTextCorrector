#ifndef GRAPHSEARCHER_H
#define GRAPHSEARCHER_H

#include <QObject>
#include "GraphElements.h"

class GraphSearcher: public QObject
{
  Q_OBJECT
public:
  GraphSearcher();

  QVector<NodeVector> findPathsBetweenTwoNodes(Node* start, Node* end, const int nodeCountBetween, const StringVector& words, const QVector<int>& sentenceId);
  QVector<NodeVector> findPathsBeforeNode(Node* start, const int nodeCountBetween, const StringVector& words = StringVector());
  QVector<NodeVector> findPathsAfterNode(Node* start, const int nodeCountBetween, const StringVector& words = StringVector());

  void searchGraphDFS(Node* end, const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words);
  void searchGraphDFSBackward(const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words);
  void searchGraphDFSForward(const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words);

  QVector<NodeVector> findBestNodeVectors(QVector<NodeVectorWeighted> &paths);

  bool isSimilarWord(QString left, QString right);

  void findSimilarWord(const int length, const QString &word, const QString &searchedWord);

  Node* findNodeWithSimilarWord(const QString& word, NodeVector& vec);

signals:
  void valueChanged(int);

private:
  QVector<int> currentSentenceId;

};

#endif // GRAPHSEARCHER_H
