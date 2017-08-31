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

  QString searchForSentence(const QString &sentence);
  QString hintSentence(const QString& sentence);
  static QString findSentenceEndingMark(const QString& line);

  void loadLanguage(const QString& fileName, const QString& lang);
  void changeLanguage(const QString& fileName);

  QString getCurrentLang() const;

signals:
  void currentLanguageChanged();

private:
  NodeVector& fixSentence(NodeVector& vector, const QStringList& sentence);
  int findSentenceIdx(NodeVector& vector);

  static const StringVector _sentenceEndings;

  GraphSearcher _graphSearcher;
  GraphParser _graphParser;

  QString _currentLang;
};

#endif // TEXTCORRECTOR_H
