#ifndef GRAPHELEMENTS_H
#define GRAPHELEMENTS_H

#include <QPair>
#include <QHash>
#include <QVector>
#include <QString>

typedef QVector<QString> StringVector;
struct Node;

struct Edge {
  int value;
  Node *nodeStart, *nodeEnd;
};

struct Node {
  QVector<int> sentenceIndexes;
  QString word;
  QHash<QString, Edge*> edgesOut, edgesIn;
};

typedef QVector<Node*> NodeVector;
typedef QPair<NodeVector, int> NodeVectorWeighted;
typedef QVector<Edge*> EdgeVector;

typedef QHash<QString, Node*> Graph;

#endif // GRAPHELEMENTS_H
