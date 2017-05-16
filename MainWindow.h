#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTextEdit;

struct Node;
struct Edge {
  int value;
  Node *nodeFrom, *nodeTo;
};

struct Node {
  QString word;
  QHash<QString, Edge*> edgesFrom, edgesTo;
};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void onEditorTextChanged();

private:
  QString findSentenceEnding(const QString& line);
  void parseToGraph(QString& sentence);

  Node* setupConnection(Node* nodeLeft, const QString& nodeRightWord);

  void loadTextsToGraph();

  int debugCounter;

  QTextEdit *_textEditor, *_resultEditor;
  const QVector<QString> _sentenceEndings;
  QHash<QString, Node*> _graph;
};

#endif // MAINWINDOW_H
