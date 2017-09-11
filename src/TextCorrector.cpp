#include "TextCorrector.h"

#include <QElapsedTimer>
#include <QDebug>
#include <QStringList>
#include "TimeElapser.h"
#include "MainWindow.h"
#include <QtConcurrent/QtConcurrent>

const StringVector TextCorrector::_sentenceEndings = StringVector() << "." << "!" << "?";
TextCorrector::TextCorrector()
{
  connect(&_graphParser, &GraphParser::finishedLoadingGraph, this, &TextCorrector::currentLanguageChanged);
  connect(this, &TextCorrector::finishedWork, MainWindow::instance(), &MainWindow::updateResult);
}

void TextCorrector::loadLanguage(const QString &fileName, const QString &lang)
{
  _currentLang = lang;

  _graphParser.loadTextsToGraph(fileName);
}

void TextCorrector::changeLanguage(const QString &fileName)
{
  _graphParser.deleteGraph();
  _currentLang = fileName.split("/").last();
  _currentLang.remove(".txt");

  _graphParser.loadTextsToGraph(fileName);
}

QString TextCorrector::getCurrentLang() const
{
  return _currentLang;
}

void TextCorrector::hintSentenceWithGraph(const QString &sentence)
{
  QString sentence_tmp = sentence;
  QStringList words = sentence_tmp.split(' ');

  // trimming and removing empty words
  for (int i = 0 ; i < words.count(); i++)
    words[i] = words.at(i).trimmed();

  Node *perviousNode = _graphParser.getGraph()[words.at(words.count() - 2)];
  if (!perviousNode)
  {
    emit finishedWork(QStringLiteral("Not found any hint."));
    return;
  }

  QProgressDialog *progress = new QProgressDialog(QStringLiteral("Searching for hint.."), QString(), 0, perviousNode->edgesOut.count(), MainWindow::instance());
  progress->setCancelButton(0);
  progress->setWindowTitle(tr(""));
  progress->setWindowModality(Qt::WindowModal);
  connect(&_graphSearcher, &GraphSearcher::valueChanged, progress, &QProgressDialog::setValue);
  progress->show();

  connect(this, &TextCorrector::finishedWork, progress, &QProgressDialog::deleteLater);

  QtConcurrent::run(this, &TextCorrector::hintSentence, perviousNode, words);

}

void TextCorrector::hintSentence(Node *node, const QStringList &words)
{
  QVector<NodeVector> alternativeNodes = _graphSearcher.findPathsAfterNode(node, 1, StringVector() << words.last());

  if (alternativeNodes.count() > 0)
  {
    QStringList res;
    Q_FOREACH(const NodeVector &vec, alternativeNodes)
    {
      QStringList words_tmp = words;
      for (int i = 1 ; i < vec.length(); i++)
      {
        if (words_tmp[words_tmp.length()-1] != vec.at(i)->word)
        {
          words_tmp[words.length()-1] = "<font color=\"green\">" + vec.at(i)->word + "</font>";
        }
      }
      res.append(words_tmp.join(" "));
    }
    emit finishedWork(res.join("<br>"));
  }
  else
    emit finishedWork(QStringLiteral("No hint was found!"));
}

void TextCorrector::searchForSentence(const QString &sentence)
{
  QString sentence_tmp = sentence;
  TimeElapser::instance()->start();
  // checking for ending mark
  const QString ending = TextCorrector::findSentenceEndingMark(sentence_tmp);
  sentence_tmp.remove(ending);

  QStringList words = sentence_tmp.split(' ');
  if (!ending.isEmpty())
    words.removeAll(ending);

  // trimming and removing empty words
  for (int i = 0 ; i < words.count(); i++)
    words[i] = words.at(i).trimmed();
  words.removeAll(QString());

  NodeVector sentenceNodes;
  // getting first node
  Node* node = 0;

  // searching for all nodes with words from sentence
  bool foundAll = true;
  for (int i = 0 ; i < words.count(); i++)
  {
    if (node)
    {
      if (node->edgesOut.contains(words[i]))
        node = node->edgesOut[words[i]]->nodeEnd;
      else
        node = 0;
    }
    else
      node = _graphParser.getGraph()[words.at(i)];

    if (!node)
      foundAll = false;

    sentenceNodes.push_back(node);
  }

  // adding ending node to end of sentence if not found
  if (!ending.isEmpty() && sentenceNodes.last() == 0)
  {
//    Node* endingNode = _graphParser.getGraph()[ending];
//    _graphParser.setupConnection(node, endingNode);
//    sentenceNodes[sentenceNodes.length() - 1] = endingNode;
  }

  if (!foundAll)
  {
    QProgressDialog *progress = new QProgressDialog(QStringLiteral("Fixing sentence.."), QString(), 0, sentenceNodes.count(), MainWindow::instance());
    progress->setCancelButton(0);
    progress->setWindowTitle(tr(""));
    progress->setWindowModality(Qt::WindowModal);
    connect(this, &TextCorrector::valueChanged, progress, &QProgressDialog::setValue);
    progress->show();

    connect(this, &TextCorrector::finishedWork, progress, &QProgressDialog::deleteLater);

    QtConcurrent::run(this, &TextCorrector::fixSentence, sentenceNodes, words, sentence);
  }
  else
  {
    emit finishedWork(parseFixedResultSentence(sentenceNodes,words));
  }
}

void TextCorrector::fixSentence(const NodeVector &vector, const QStringList &words, const QString &sentence)
{
  QVector<NodeVector> fixes = fixSentenceWithGraph(vector, words);
  if (fixes.isEmpty())
  {
    emit finishedWork(parseFixedResultSentence(vector,words));
    return;
  }

  QStringList fixesForSent;
  int i = 0;
  Q_FOREACH(const NodeVector &vec, fixes)
  {
    emit valueChanged(++i);
    QString res = parseFixedResultSentence(vec, words).trimmed();

    // highlighting words that were corrected
    QString temporarySentence = sentence;
    QStringList first = temporarySentence.replace(".", " .").replace(",", " ,").replace("  ", " ").trimmed().split(" ");
    QStringList corrected = res.split(" ");

    for (int i = 0 ; i < first.length(); i++)
    {
      if (i == corrected.length())
        corrected.append(first.at(i));
    }

    res = corrected.join(" ").replace(" .", ".").replace(" ,", ",").replace("  ", " ").trimmed();
    fixesForSent.append(res);
  }
#ifdef DEBUG
    qDebug() << ">>> FIXED in " << TimeElapser::instance()->elapsed() << "ms <<<" << fixesForSent;
#endif
  emit finishedWork(fixesForSent.join("<br>"));
}

QVector<NodeVector> TextCorrector::fixSentenceWithGraph(const NodeVector &vector, const QStringList& sentence)
{
  QVector<NodeVector> res;
  QVector<int> sentenceIdx = findSentenceIdx(vector);
#ifdef DEBUG
  qDebug() << "\nFIXING" << sentence.join(' ') << " | sent idx = " << sentenceIdx;
#endif

  // length between pervious and next node
  int edgeLength = 1;
  for (int i = 0 ; i < vector.count(); i+=edgeLength)
  {
    edgeLength = 1;
    if (vector.at(i) == 0)
    {
      // any next node after
      Node* nextNode = 0;
      // searching for next node
      for (int j = i+1 ; j < vector.count(); j++)
      {
        edgeLength++;
        if (vector.at(j))
        {
          nextNode = vector.at(j);
          break;
        }
      }

      if (i == 0) // fixing first words
      {
        if (nextNode) // something in sentence is correct
        {
          StringVector wordsToFix;
          for(int k = i+edgeLength -2; k > -1; k--)
            wordsToFix.push_back(sentence.at(k));

          QVector<NodeVector> alternativeNodes = _graphSearcher.findPathsBeforeNode(nextNode, edgeLength-1, wordsToFix);

          if (alternativeNodes.count() > 0)
          {
            Q_FOREACH(NodeVector vec, alternativeNodes)
            {
              vec.pop_front();
              NodeVector fix = vector;
              for (int k = 0 ; k < vec.length(); k++)
                fix[i+k] = vec[vec.length() - k - 1];
              res.push_back(fix);
            }
          }
#ifdef DEBUG
          qDebug() << "fixing" << (edgeLength-1) << "---" << nextNode->word;
#endif
        }
        else // whole sentence is wrong
        {

          res.push_back(vector);
          return res;
          // TODO handling whole sentence error
        }
      }
      else // fixing any words between two found
      {
        // node before searched one
        Node* perviousNode = vector.at(i-1);

        if (nextNode) // if only few words are incorrect
        {
#ifdef DEBUG
          qDebug() << "fixing between" << perviousNode->word << "---" << (edgeLength-1) << "---" << nextNode->word;
#endif
          StringVector wordsToFix;
          for(int k = i ; k < i + edgeLength; k++)
            wordsToFix.push_back(sentence.at(k));

          QVector<NodeVector> alternativeNodes = _graphSearcher.findPathsBetweenTwoNodes(perviousNode, nextNode, edgeLength, wordsToFix, sentenceIdx);

          if (alternativeNodes.count() > 0)
          {
            Q_FOREACH(NodeVector vec, alternativeNodes)
            {
              vec.pop_front();
              vec.pop_back();
              NodeVector fix = vector;
              for (int k = 0 ; k < vec.length(); k++)
                fix[i+k] = vec[k];
              res.push_back(fix);
            }
          }
        }
        else // fixing last words
        {
          StringVector wordsToFix;
          for(int k = i ; k < i + edgeLength; k++)
            wordsToFix.push_back(sentence.at(k));
          QVector<NodeVector> alternativeNodes = _graphSearcher.findPathsAfterNode(perviousNode, edgeLength, wordsToFix);

          if (alternativeNodes.count() > 0)
          {
            Q_FOREACH(NodeVector vec, alternativeNodes)
            {
              vec.pop_front();
              NodeVector fix = vector;
              for (int k = 0 ; k < vec.length(); k++)
                fix[i+k] = vec[k];
              res.push_back(fix);
            }
          }
        }

      }
    }
  }
  return res;
}

QString TextCorrector::findSentenceEndingMark(const QString &line)
{
  Q_FOREACH(const QString& end, _sentenceEndings)
  {
    if (line.contains(end))
      return end;
  }
  return QString();
}

QVector<int> TextCorrector::findSentenceIdx(const NodeVector &vector)
{
  QVector<int> indexes;
  Q_FOREACH(Node* node, vector)
  {
    if (node)
    {
      if (indexes.length() == 0)
      {
        indexes = node->sentenceIndexes;
        continue;
      }
      else
      {
        QVector<int> toRemove;
        Q_FOREACH(const int idx, indexes)
        {
          if (!node->sentenceIndexes.contains(idx))
            toRemove.push_back(idx);
        }

        Q_FOREACH(const int i, toRemove)
        {
          indexes.removeAll(i);
        }
      }
    }
  }

  return indexes;
}

QString TextCorrector::parseFixedResultSentence(const NodeVector &vector, const QStringList &sentence)
{
  QString res;
  for (int i = 0 ; i < vector.length(); i++)
  {
    Node *n = vector.at(i);
    if (n)
    {
        if (n->word != sentence.at(i))
            res += "<font color=\"red\">"+ n->word + "</font> ";
        else
            res += n->word + " ";
    }
    else
      res += "<u><font color=\"#B2B200\">" +sentence.at(i) +"</font></u> ";
  }
  return res;
}
