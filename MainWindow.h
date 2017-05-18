#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTextEdit;

struct Node;
struct Edge {
  int value;
  Node *nodeStart, *nodeEnd;
};

struct Node {
  QString word;
  QHash<QString, Edge*> edgesOut, edgesIn;
};

typedef QVector<Node*> NodeVector;
typedef QVector<Edge*> EdgeVector;
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void onEditorTextChanged();

private:
  QString searchForSentence(QString sentence);

  Node* findNodeWithSimilarWord(const QString& word, NodeVector& vec);

  QVector<NodeVector> findPathsBetweenTwoNodes(Node* start, Node* end, const int nodeCountBetween);

  void searchGraphDFS(Node* end, const int maxDepth, QStack<Node*>& stack, QVector<NodeVector>& paths);

  NodeVector& fixSentence(NodeVector& vector, const QStringList& sentence);

  QString findSentenceEndingMark(const QString& line);
  void parseToGraph(QString& sentence);

  Node* setupConnection(Node* nodeLeft, const QString& nodeRightWord);
  Node* setupConnection(Node* nodeLeft, Node* nodeRight);

  void loadTextsToGraph(const QString& fileName);

  int debugEdgeCounter;

  QTextEdit *_textEditor, *_resultEditor;
  const QVector<QString> _sentenceEndings;
  QHash<QString, Node*> _graph;
};

#endif // MAINWINDOW_H
