#ifndef TEXTCORRECTOR_H
#define TEXTCORRECTOR_H

#include "GraphSearcher.h"
#include "GraphParser.h"

class TextCorrector
{
public:
  TextCorrector();

  QString searchForSentence(QString sentence);
  static QString findSentenceEndingMark(const QString& line);

private:
  NodeVector& fixSentence(NodeVector& vector, const QStringList& sentence);

  static const StringVector _sentenceEndings;

  GraphSearcher _graphSearcher;
  GraphParser _graphParser;
};

#endif // TEXTCORRECTOR_H
