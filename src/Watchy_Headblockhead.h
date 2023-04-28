#ifndef WATCHY_HEADBLOCKHEAD_H
#define WATCHY_HEADBLOCKHEAD_H

#include <Watchy.h>

#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Regular_39.h"
#include "Seven_Segment10pt7b.h"
#include "icons.h"

typedef struct watchyHeadblockheadSettings {
  bool is12Hrs;
  float sunRiseTime;
  float sunSetTime;
} watchyHeadblockheadSettings;

class WatchyHeadblockhead : public Watchy {
  using Watchy::Watchy;

public:
  watchyHeadblockheadSettings *headblockheadSettings;
  void drawWatchFace();
  void getSunriseSunset(float *sunRiseTime, float *sunSetTime);
  void drawSunriseSunset(int x, int y, int sunRiseTime, int sunSetTime);
  void drawTime(int x, int y);
  void drawDate(int x, int y);
  void drawSteps(int x, int y, uint32_t step);
  //  void drawWeather();
  void drawBattery(int x, int y);
  void drawWifi(int x, int y);
  void drawBluetooth(int x, int y);
  virtual void handleButtonPress(); // override default button press handling
};

#endif
