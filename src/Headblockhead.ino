#include "Watchy_Headblockhead.h"
#include "settings.h"

WatchyHeadblockhead watchy(settings);

// RTC_DATA_ATTR tells the compiler to store the variable in RTC slow memory - it will stay there when the watch is in deep sleep
RTC_DATA_ATTR watchyHeadblockheadSettings extraSettings{
    .is12Hrs = false,
    .sunRiseTime = 0,
    .sunSetTime = 0,
.arborattendance = -1.00,
.arborpoints = -1,
.arbortimetableEvent0 = {0},
.arbortimetableEvent1 = {0},
.arbortimetableEvent2 = {0},
.arbortimetableEvent3 = {0},
.arbortimetableEvent4 = {0},
.arbortimetableEvent5 = {0},
.arbortimetableEvent6 = {0},
.arbortimetableEvent7 = {0},
.arbortimetableEvent8 = {0},
.arbortimetableEvent9 = {0},
   .arborweek = "0",
    .state = Home,
    .currentMenuItem = 0,
};

void setup(){
    Serial.begin(9600);
    Serial.flush();
watchy.headblockheadSettings = &extraSettings;
watchy.arborURL = ARBOR_API_URL;
watchy.arborAPIKey = ARBOR_API_KEY;
  watchy.init();
}

void loop(){
}



