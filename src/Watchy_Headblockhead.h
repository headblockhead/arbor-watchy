#ifndef WATCHY_HEADBLOCKHEAD_H
#define WATCHY_HEADBLOCKHEAD_H

#include <Watchy.h>

#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Regular_39.h"
#include "Ramabhadra_Regular25pt7b.h"
#include "Ramabhadra_Regular4pt7b.h"
#include "Ramabhadra_Regular7pt7b.h"
#include "Seven_Segment10pt7b.h"
#include "icons.h"

enum watchyHeadblockheadState { Home, Menu, ArborAlert };

typedef struct arborTimetableEvent {
  int eventStart;
  int eventEnd;
  String eventEvent;
  String eventLocation;
} arborTimetableEvent;

typedef struct watchyHeadblockheadSettings {
  bool is12Hrs;
  int sunRiseTime;
  int sunSetTime;
  double arborattendance;
  int arborpoints;
  uint32_t arbortimetableEvent0[74];
  uint32_t arbortimetableEvent1[74];
  uint32_t arbortimetableEvent2[74];
  uint32_t arbortimetableEvent3[74];
  uint32_t arbortimetableEvent4[74];
  uint32_t arbortimetableEvent5[74];
  uint32_t arbortimetableEvent6[74];
  uint32_t arbortimetableEvent7[74];
  uint32_t arbortimetableEvent8[74];
  uint32_t arbortimetableEvent9[74];
  String arborweek;
  watchyHeadblockheadState state;
  int currentMenuItem;
} watchyHeadblockheadSettings;

class WatchyHeadblockhead : public Watchy {
  using Watchy::Watchy;

public:
  watchyHeadblockheadSettings *headblockheadSettings;
  String arborURL;
  String arborAPIKey;
  void timetableToTemp();
  void tempToTimetable();
  void singleTimetableToEvent(uint32_t timetable[74], arborTimetableEvent *event);
  void singleEventToTimetable(uint32_t (*timetable)[74], arborTimetableEvent event);
  void getArbor(double *attendance, int *points, std::vector<arborTimetableEvent> *timetable, String *week);
  void drawWatchFace();
  void drawHomeScreen();
  void drawMenuScreen();
  void drawArborAlertScreen(int eventStart, int eventEnd, String eventLocation, String eventEvent);
  void getSunriseSunset(int *sunRiseTime, int *sunSetTime);
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
