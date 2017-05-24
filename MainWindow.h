#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTextEdit;


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
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void onEditorTextChanged();
  void onCheckSpelling();

private:
  QString searchForSentence(QString sentence);

  Node* findNodeWithSimilarWord(const QString& word, NodeVector& vec);
  bool isSimilarWord(QString left, QString right);

  QVector<NodeVectorWeighted> findPathsBetweenTwoNodes(Node* start, Node* end, const int nodeCountBetween, const StringVector& words = StringVector());
  QVector<NodeVectorWeighted> findPathsBeforeNode(Node* start, const int nodeCountBetween, const StringVector& words = StringVector());
  QVector<NodeVectorWeighted> findPathsAfterNode(Node* start, const int nodeCountBetween, const StringVector& words = StringVector());

  void searchGraphDFS(Node* end, const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words);
  void searchGraphDFSBackward(const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words);
  void searchGraphDFSForward(const int maxDepth,const int currentWeight, QStack<Node*>& stack, QVector<NodeVectorWeighted>& paths, const StringVector& words);

  NodeVector& fixSentence(NodeVector& vector, const QStringList& sentence);

  QString findSentenceEndingMark(const QString& line);
  void parseToGraph(QString& sentence);

  NodeVector findBestNodeVector(const QVector<NodeVectorWeighted>& paths);

  Node* setupConnection(Node* nodeLeft, const QString& nodeRightWord, const int sentenceIdx);
  Node* setupConnection(Node* nodeLeft, Node* nodeRight);

  void loadTextsToGraph(const QString& fileName);

  int debugEdgeCounter;

  QTextEdit *_textEditor, *_resultEditor;
  const StringVector _sentenceEndings;
  QHash<QString, Node*> _graph;

  int _sentenceIndexCounter;
};

#endif // MAINWINDOW_H
