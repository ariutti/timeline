/*
*  The "Touch" class
* Created by Nicola Ariutti, May 17, 2019
* CC0
*/
#ifndef _NA_TOUCH_
#define _NA_TOUCH_
#include "Arduino.h"

class Touch
{
public:
  Touch() {};

  void addIndex(int _index);
  void removeIndex(int _index);
  int getInterpolatedIndex();

private:
  enum {
    EMPTY = 0,
    SINGLETOUCH,
    DOUBLETOUCH,
  } touchStatus = EMPTY;
  int newIdx = -1;
  int oldIdx = -1;
};
#endif
