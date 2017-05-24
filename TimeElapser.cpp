#include "TimeElapser.h"

TimeElapser* TimeElapser::_instance = 0;
TimeElapser::TimeElapser()
  : QElapsedTimer()
{

}

TimeElapser* TimeElapser::instance()
{
  if (!_instance)
    _instance = new TimeElapser();
  return _instance;
}
