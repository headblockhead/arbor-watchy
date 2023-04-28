#include "Watchy_Headblockhead.h"
#include "settings.h"

WatchyHeadblockhead watchy(settings);

// RTC_DATA_ATTR tells the compiler to store the variable in RTC slow memory - it will stay there when the watch is in deep sleep
RTC_DATA_ATTR watchyHeadblockheadSettings extraSettings{
    .is12Hrs = false,
    .sunRiseTime = 0,
    .sunSetTime = 0,
};

void setup(){
    Serial.begin(9600);
    Serial.flush();
watchy.headblockheadSettings = &extraSettings;
  watchy.init();
}

void loop(){
}



