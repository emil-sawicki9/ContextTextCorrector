#ifndef TIMEELAPSER_H
#define TIMEELAPSER_H

#include <QElapsedTimer>

class TimeElapser : public QElapsedTimer
{
public:
  static TimeElapser* instance();

private:
  TimeElapser();
  static TimeElapser* _instance;
};

#endif // TIMEELAPSER_H
