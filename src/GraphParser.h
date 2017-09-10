#ifndef GRAPHPARSER_H
#define GRAPHPARSER_H

#include <QObject>
#include <QFile>

#include "GraphElements.h"

class GraphParser : public QObject
{
  Q_OBJECT
public:
  GraphParser();

  const Graph& getGraph() const;

  void loadTextsToGraph(const QString& fileName);

  void deleteGraph();

  Node* setupConnection(Node* nodeLeft, Node* nodeRight);

signals:
  void finishedLoadingGraph();
  void valueChanged(int);


private:
  void createGraph(QByteArray &text);

  void parseToGraph(QString& sentence);

  Node* setupConnection(Node* nodeLeft, const QString& nodeRightWord, const int sentenceIdx);

  int _sentenceIndexCounter;
  int debugEdgeCounter, debugEdgeMaxCounter;
  QString debugMostEdgeWord;
  Graph _graph;
};

#endif // GRAPHPARSER_H
