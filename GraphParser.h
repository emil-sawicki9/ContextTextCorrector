#ifndef GRAPHPARSER_H
#define GRAPHPARSER_H

#include "GraphElements.h"

class GraphParser
{
public:
  GraphParser();

  const Graph& getGraph() const;

  void loadTextsToGraph();

  Node* setupConnection(Node* nodeLeft, Node* nodeRight);
private:
  void parseToGraph(QString& sentence);

  Node* setupConnection(Node* nodeLeft, const QString& nodeRightWord, const int sentenceIdx);

  int _sentenceIndexCounter;
  int debugEdgeCounter;
  Graph _graph;
};

#endif // GRAPHPARSER_H
