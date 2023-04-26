#ifndef WATCHY_HEADBLOCKHEAD_H
#define WATCHY_HEADBLOCKHEAD_H

#include <Watchy.h>

#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Regular_39.h"
#include "Seven_Segment10pt7b.h"
#include "icons.h"

class WatchyHeadblockhead : public Watchy {
  using Watchy::Watchy;

public:
  void drawWatchFace();
  void drawTime();
  void drawDate();
  void drawSteps();
  void drawWeather();
  void drawBattery();
  void drawWifi(int x, int y);
  void drawBluetooth(int x, int y);
  virtual void handleButtonPress(); // override default button press handling
};

#endif
