#ifndef TEXTCORRECTOR_H
#define TEXTCORRECTOR_H

#include <QObject>

#include "GraphSearcher.h"
#include "GraphParser.h"

class TextCorrector: public QObject
{
  Q_OBJECT
public:
  TextCorrector();

  void searchForSentence(const QString &sentence);
  void hintSentenceWithGraph(const QString& sentence);
  static QString findSentenceEndingMark(const QString& line);

  void changeLanguage(const QString& fileName);

  QString getCurrentLang() const;

  void loadLanguage(const QString& fileName, const QString& lang);

signals:
  void currentLanguageChanged();
  void valueChanged(int);
  void finishedWork(QString);

private:
  QString parseFixedResultSentence(const NodeVector &vector, const QStringList& sentence);
  void hintSentence(Node* node, const QStringList &words);
  void fixSentence(const NodeVector &vector,const QStringList &words, const QString &sentence);
  QVector<NodeVector> fixSentenceWithGraph(const NodeVector& vector, const QStringList& sentence);
  QVector<int> findSentenceIdx(const NodeVector& vector);

  static const StringVector _sentenceEndings;

  GraphSearcher _graphSearcher;
  GraphParser _graphParser;

  QString _currentLang;
};

#endif // TEXTCORRECTOR_H
