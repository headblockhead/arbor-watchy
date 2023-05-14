#include "Watchy_Headblockhead.h"

std::vector<String> menuItems = {"Open Watchy Menu", "Toggle 12/24H"};

void WatchyHeadblockhead::handleButtonPress() {

  if (guiState != WATCHFACE_STATE) { // If we are not on the watchface, don't use custom button handling.
    Watchy::handleButtonPress();
    return;
  }

  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
  if (wakeupBit) {
    RTC.read(currentTime); // Update the current time.
  }

  if (wakeupBit & MENU_BTN_MASK) {
    vibMotor(10, 10);
    if (headblockheadSettings->state == Menu) {
      // Click a menu item.
      if (headblockheadSettings->currentMenuItem == 0) {
        // First item: open Watchy menu.
        Watchy::showMenu(menuIndex, false);
        return;
      } else if (headblockheadSettings->currentMenuItem == 1) {
        // Second item: toggle 12/24 hour mode.
        headblockheadSettings->is12Hrs = !headblockheadSettings->is12Hrs;
        drawWatchFace();
        Watchy::showWatchFace(true);
        return;
      }
    } else if (headblockheadSettings->state == Home) {
      // If we are on the home screen, open the menu.
      headblockheadSettings->state = Menu;
      drawWatchFace();
      Watchy::showWatchFace(false);
      return;
    } else if (headblockheadSettings->state == ArborAlert) {
      // If we are on the arbor alert screen, go back to the home screen.
      headblockheadSettings->state = Home;
      drawWatchFace();
      Watchy::showWatchFace(false);
      return;
    } else {
      // Otherwise, pass the button press to Watchy.
      Watchy::handleButtonPress();
      return;
    }
    return;
  } else if (wakeupBit & UP_BTN_MASK) {
    vibMotor(10, 10);
    if (headblockheadSettings->state == Menu) {
      // If we are in the menu, move the cursor up.
      headblockheadSettings->currentMenuItem--;
      if (headblockheadSettings->currentMenuItem < 0) {
        // If we are at the top of the menu, loop back to the bottom.
        headblockheadSettings->currentMenuItem = menuItems.size() - 1;
      }
      drawWatchFace();             // Redraw the watchface.
      Watchy::showWatchFace(true); // Apply the changes - true for partial refresh.
      return;
    } else if (headblockheadSettings->state == Home) {
      // If we are on the home screen, do nothing.
    } else if (headblockheadSettings->state == ArborAlert) {
      // If we are on the Arbor Alert screen, do nothing.
    } else {
      // Otherwise, pass the button press to the default handler.
      Watchy::handleButtonPress();
      return;
    }
  } else if (wakeupBit & DOWN_BTN_MASK) {
    vibMotor(10, 10);
    if (headblockheadSettings->state == Menu) {
      // If on the menu, move down the menu.
      headblockheadSettings->currentMenuItem++;
      if (headblockheadSettings->currentMenuItem > menuItems.size() - 1) {
        // If we are at the bottom of the menu, go to the top.
        headblockheadSettings->currentMenuItem = 0;
      }
      drawWatchFace();             // Redraw the watchface.
      Watchy::showWatchFace(true); // Apply the changes - true for partial refresh.
      return;
    } else if (headblockheadSettings->state == Home) {
      // If on the home screen, do nothing.
    } else if (headblockheadSettings->state == ArborAlert) {
      // If on the arbor alert screen, do nothing.
    } else {
      // Otherwise, pass the button press to the default handler.
      Watchy::handleButtonPress();
      return;
    }
  } else if (wakeupBit & BACK_BTN_MASK) {
    vibMotor(10, 10);
    if (headblockheadSettings->state == Menu) {
      // If on the menu, go back to the home screen.
      headblockheadSettings->state = Home;
      drawWatchFace();              // Redraw the watchface.
      Watchy::showWatchFace(false); // Apply the changes - false for full refresh.
      return;
    } else if (headblockheadSettings->state == Home) {
      // If on the home screen, to nothing.
    } else if (headblockheadSettings->state == ArborAlert) {
      // Go back to the home screen.
      headblockheadSettings->state = Home;
      drawWatchFace();              // Redraw the watchface.
      Watchy::showWatchFace(false); // Apply the changes - false for full refresh.
    } else {
      // Otherwise, let the default button handling take over.
      Watchy::handleButtonPress();
      return;
    }
  } else { // No buttons pressed.
    if (headblockheadSettings->state == ArborAlert) {
      headblockheadSettings->state = Menu;
      drawWatchFace();
      Watchy::showWatchFace(false);
      return;
    }
  }
  drawWatchFace();             // Redraw the watchface.
  Watchy::showWatchFace(true); // Apply the changes - true for partial refresh.
  return;
}

bool isDark = false;
std::vector<arborTimetableEvent> tempTimetable;

void WatchyHeadblockhead::singleTimetableToEvent(uint32_t timetable[74], arborTimetableEvent *event) {
  event->eventStart = timetable[0];
  event->eventEnd = timetable[1];
  for (int j = 0; j < 21; j++) {
    if (timetable[2 + j] != 0) {
      event->eventLocation += (char)timetable[2 + j];
    }
  }
  for (int j = 0; j < 50; j++) {
    if (timetable[23 + j] != 0) {
      event->eventEvent += (char)timetable[23 + j];
    }
  }
}

void WatchyHeadblockhead::timetableToTemp() {
  tempTimetable.clear();
  for (int i = 0; i < 10; i++) {
    // For all 10 possible events, get the data.
    // The hard part - translate from a list of 74 integers to an arborTimetableEvent.

    // Integer 0: the seconds since midnight when the event starts.
    // Integer 1: the seconds since midnight when the event ends.
    // Integer 2-22: the character codes for the event location. Theese need to be translated to a String.
    // Integer 23-73: the character codes for the event name. Theese need to be translated to a String.

    arborTimetableEvent newEvent = arborTimetableEvent();

    if (i == 0) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent0, &newEvent);
    } else if (i == 1) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent1, &newEvent);
    } else if (i == 2) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent2, &newEvent);
    } else if (i == 3) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent3, &newEvent);
    } else if (i == 4) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent4, &newEvent);
    } else if (i == 5) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent5, &newEvent);
    } else if (i == 6) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent6, &newEvent);
    } else if (i == 7) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent7, &newEvent);
    } else if (i == 8) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent8, &newEvent);
    } else if (i == 9) {
      singleTimetableToEvent(headblockheadSettings->arbortimetableEvent9, &newEvent);
    }

    // Add the event to the temporary timetable.
    tempTimetable.push_back(newEvent);
  }
}

void WatchyHeadblockhead::singleEventToTimetable(uint32_t (*timetable)[74], arborTimetableEvent event) {
  (*timetable)[0] = event.eventStart;
  (*timetable)[1] = event.eventEnd;
  for (int j = 0; j < 21; j++) {
    (*timetable)[2 + j] = event.eventLocation[j];
  }
  for (int j = 0; j < 50; j++) {
    (*timetable)[33 + j] = event.eventEvent[j];
  }
}
void WatchyHeadblockhead::tempToTimetable() {
  for (int i = 0; i < tempTimetable.size(); i++) {
    if (i == 0) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent0, tempTimetable[i]);
    } else if (i == 1) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent1, tempTimetable[i]);
    } else if (i == 2) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent2, tempTimetable[i]);
    } else if (i == 3) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent3, tempTimetable[i]);
    } else if (i == 4) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent4, tempTimetable[i]);
    } else if (i == 5) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent5, tempTimetable[i]);
    } else if (i == 6) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent6, tempTimetable[i]);
    } else if (i == 7) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent7, tempTimetable[i]);
    } else if (i == 8) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent8, tempTimetable[i]);
    } else if (i == 9) {
      singleEventToTimetable(&headblockheadSettings->arbortimetableEvent9, tempTimetable[i]);
    }
  }
}

void WatchyHeadblockhead::drawWatchFace() {
  // If after sunset, switch to night mode.

  // If sunrise/sunset data is not yet set, or it is the first minute of every hour, update the sunrise/sunset data.
  if ((currentTime.Minute == 1) || headblockheadSettings->sunRiseTime == 0 || headblockheadSettings->sunSetTime == 0) {
    getSunriseSunset(&headblockheadSettings->sunRiseTime, &headblockheadSettings->sunSetTime);
  }

  // If the current time is after sunset, or before sunrise, switch to night mode.
  isDark = ((currentTime.Second) + (60 * currentTime.Minute) + (60 * 60 * currentTime.Hour)) < headblockheadSettings->sunRiseTime || ((currentTime.Second) + (60 * currentTime.Minute) + (60 * 60 * currentTime.Hour)) > headblockheadSettings->sunSetTime;

  // Set the colors to use.
  display.fillScreen(isDark ? GxEPD_BLACK : GxEPD_WHITE);
  display.setTextColor(isDark ? GxEPD_WHITE : GxEPD_BLACK);

  // If the arbor data is not yet set, or it is the first minute of every hour, update the arbor data.
  if ((currentTime.Minute == 1) || headblockheadSettings->arborattendance == -1.00 || headblockheadSettings->arborpoints == -1) {
    getArbor(&headblockheadSettings->arborattendance, &headblockheadSettings->arborpoints, &tempTimetable, &headblockheadSettings->arborweek);
    tempToTimetable();
  } else {
    timetableToTemp();
  }

  Serial.println("attendance: " + String(headblockheadSettings->arborattendance));
  Serial.println("points: " + String(headblockheadSettings->arborpoints));

  int currentArborEventStart = 0;
  int currentArborEventEnd = 0;
  String currentArborEventLocation = "0";
  String currentArborEventEvent = "0";

  for (int i = 0; i < tempTimetable.size(); i++) {
    Serial.println("event " + String(i) + ": " + String(tempTimetable[i].eventStart) + " " + String(tempTimetable[i].eventEnd) + " " + String(tempTimetable[i].eventLocation) + " " + String(tempTimetable[i].eventEvent));
    if (tempTimetable[i].eventEnd == 0) {
      continue;
    }
    if (((currentTime.Second) + (60 * currentTime.Minute) + (60 * 60 * currentTime.Hour)) == tempTimetable[i].eventStart) {
      Serial.println("event " + String(i) + " starts now!");
      currentArborEventStart = tempTimetable[i].eventStart;
      currentArborEventEnd = tempTimetable[i].eventEnd;
      currentArborEventLocation = String(tempTimetable[i].eventLocation);
      currentArborEventEvent = String(tempTimetable[i].eventEvent);
      headblockheadSettings->state = ArborAlert;
    }
  }

  // If we are on the home screen, draw the home screen.
  if (headblockheadSettings->state == Home) {
    drawHomeScreen();
  } else if (headblockheadSettings->state == Menu) {
    drawMenuScreen();
  } else if (headblockheadSettings->state == ArborAlert) {
    vibMotor(100, 20);
    drawArborAlertScreen(currentArborEventStart, currentArborEventEnd, currentArborEventLocation, currentArborEventEvent);
  }

  return;
}

void WatchyHeadblockhead::drawMenuScreen() {
  // Title
  display.setFont(&Ramabhadra_Regular25pt7b);
  display.setCursor(0, 37);
  display.print("Menu");

  // Decorative lines
  display.drawFastHLine(0, 43, 125, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawFastHLine(0, 44, 125, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawFastHLine(0, 45, 125, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(125, 43, 145, 23, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(125, 44, 145, 24, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(125, 45, 145, 25, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawFastHLine(145, 23, 55, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawFastHLine(145, 24, 54, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawFastHLine(145, 25, 53, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  // Top Right: Time
  display.setFont(&DSEG7_Classic_Regular_15);
  display.setCursor(143, 20);

  int displayHour;
  if (headblockheadSettings->is12Hrs) {
    displayHour = ((currentTime.Hour + 11) % 12) + 1;
  } else {
    displayHour = currentTime.Hour;
  }
  if (displayHour < 10) {
    display.print("0");
  }
  display.print(displayHour);
  display.print(":");
  if (currentTime.Minute < 10) {
    display.print("0");
  }
  display.println(currentTime.Minute);

  display.setFont(&Seven_Segment10pt7b);
  for (int i = 0; i < menuItems.size(); i++) {
    if (i == headblockheadSettings->currentMenuItem) {
      display.setCursor(15, 70 + (i * 30));
      display.print("> " + menuItems[i]);
    } else {
      display.setCursor(5, 70 + (i * 30));
      display.print("  " + menuItems[i]);
    }
  }
}

void WatchyHeadblockhead::drawHomeScreen() {
  // Screen bounds: 200x200

  // Top Left widget:
  drawBattery(2, 5);    // 40x20
  drawWifi(42, 6);      // 26x18
  drawBluetooth(72, 5); // 13x20
  // Bottom line
  display.drawLine(0, 30, 90, 30, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  // Right line
  display.drawLine(90, 30, 98, 0, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  display.drawLine(9, 33, 9, 41, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(9, 33, 1, 38, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(9, 41, 1, 38, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  display.setFont(&Ramabhadra_Regular4pt7b);
  display.setCursor(13, 40);
  display.print("CANCEL");

  // Top Right widget:
  drawSteps(111, 6, sensor.getCounter());
  // Bottom line
  display.drawLine(110, 30, 200, 30, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  // Left line
  display.drawLine(102, 0, 110, 30, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  display.drawLine(191, 33, 191, 41, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(191, 33, 199, 38, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(191, 41, 199, 38, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  display.setFont(&Ramabhadra_Regular4pt7b);
  display.setCursor(176, 40);
  display.print("UP");

  // Main time.
  drawTime(5, 110);  // 200x100
  drawDate(39, 125); // 118x25

  // Bottom Bar
  drawSunriseSunset(5, 176, headblockheadSettings->sunRiseTime, headblockheadSettings->sunSetTime); // 95x95

  // Left:
  //  Top line
  display.drawLine(0, 170, 90, 170, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  // Right line
  display.drawLine(90, 170, 98, 200, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  display.drawLine(9, 167, 9, 159, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(9, 167, 1, 162, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(9, 159, 1, 162, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  display.setFont(&Ramabhadra_Regular4pt7b);
  display.setCursor(13, 165);
  display.print("MENU");

  // Right:
  //  Top line
  display.drawLine(110, 170, 200, 170, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  // Left line
  display.drawLine(102, 200, 110, 170, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  display.drawLine(191, 167, 191, 159, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(191, 167, 199, 162, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(191, 159, 199, 162, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  display.setFont(&Ramabhadra_Regular4pt7b);
  display.setCursor(162, 165);
  display.print("DOWN");
}

void WatchyHeadblockhead::drawArborAlertScreen(int eventStart, int eventEnd, String eventLocation, String eventEvent) {
  display.setFont(&Ramabhadra_Regular7pt7b);
  display.setCursor(1, 12);
  display.println("Next Lesson!");
  display.drawFastHLine(0, 15, 200, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  if (eventLocation.length() > 4) {
    display.setFont(&Seven_Segment10pt7b);
    display.setCursor((200 - (10 * eventLocation.length())) / 2, 75);
  } else {
    display.setFont(&DSEG7_Classic_Bold_53);
    display.setCursor(13, 110);
  }
  display.println(eventLocation);

  display.setFont(&Ramabhadra_Regular7pt7b);
  display.setCursor((200 - (7 * eventEvent.length())) / 2, 130);
  display.println(eventEvent);

  display.setFont(&Seven_Segment10pt7b);
  display.setCursor(45, 175);
  char s[6];
  if (headblockheadSettings->is12Hrs) {
    sprintf(s, "%02d:%02d", ((eventStart / 3600) % 12), (eventStart / 60) % 60);
  } else {
    sprintf(s, "%02d:%02d", (eventStart / 3600) % 24, (eventStart / 60) % 60);
  }
  char e[6];
  if (headblockheadSettings->is12Hrs) {
    sprintf(e, "%02d:%02d", ((eventEnd / 3600) % 12), (eventEnd / 60) % 60);
  } else {
    sprintf(e, "%02d:%02d", (eventEnd / 3600) % 24, (eventEnd / 60) % 60);
  }
  display.print(s);
  display.print(" - ");
  display.print(e);
}

void WatchyHeadblockhead::drawBattery(int x, int y) {
  display.drawBitmap(x, y, battery, 37, 21, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.fillRect(x + 5, y + 5, 27, 11, isDark ? GxEPD_BLACK : GxEPD_WHITE); // clear battery segments
  int8_t batteryLevel = 0;
  float VBAT = getBatteryVoltage();
  if (VBAT > 4.1) {
    batteryLevel = 3;
  } else if (VBAT > 3.95 && VBAT <= 4.1) {
    batteryLevel = 2;
  } else if (VBAT > 3.80 && VBAT <= 3.95) {
    batteryLevel = 1;
  } else if (VBAT <= 3.80) {
    batteryLevel = 0;
  }

  for (int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++) {
    display.fillRect(x + 5 + (batterySegments * 9), y + 5, 7, 11, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  }
}

void WatchyHeadblockhead::drawWifi(int x, int y) {
  display.drawBitmap(x, y, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, isDark ? GxEPD_WHITE : GxEPD_BLACK);
}

void WatchyHeadblockhead::drawBluetooth(int x, int y) {
  display.drawBitmap(x, y, BLE_CONFIGURED ? bluetooth : bluetoothoff, 13, 21, isDark ? GxEPD_WHITE : GxEPD_BLACK);
}

void WatchyHeadblockhead::drawTime(int x, int y) {
  display.setFont(&DSEG7_Classic_Bold_53);
  display.setCursor(x, y);
  int displayHour;
  if (headblockheadSettings->is12Hrs) {
    displayHour = ((currentTime.Hour + 11) % 12) + 1;
  } else {
    displayHour = currentTime.Hour;
  }
  if (displayHour < 10) {
    display.print("0");
  }
  display.print(displayHour);
  display.print(":");
  if (currentTime.Minute < 10) {
    display.print("0");
  }
  display.println(currentTime.Minute);

  // Draw the AM/PM indicator.
  display.setFont(&Seven_Segment10pt7b);
  display.setCursor(x + 159, y + 18);
  if (headblockheadSettings->is12Hrs) {
    if (currentTime.Hour < 12) {
      display.println("AM");
    } else {
      display.println("PM");
    }
  } else {
    display.setCursor(x + 153, y + 16);
    display.println("24H");
  }
}

void WatchyHeadblockhead::drawDate(int x, int y) {
  display.setFont(&DSEG7_Classic_Bold_25);
  String dayOfWeek = dayShortStr(currentTime.Wday);
  String date = dayOfWeek.substring(0, 3) + " " + currentTime.Day;
  display.setCursor(x, y + 20);
  display.println(date);
}

void WatchyHeadblockhead::drawSteps(int x, int y, uint32_t step) {
  // reset step counter at midnight
  if (currentTime.Hour == 0 && currentTime.Minute == 0) {
    sensor.resetStepCounter();
  }
  display.drawBitmap(x, y - 2, steps, 19, 23, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.setFont(&DSEG7_Classic_Regular_15);
  display.setCursor(x + 23, y + 18);
  char s[6];
  sprintf(s, "%05d", step);
  display.println(s);
}

void WatchyHeadblockhead::getArbor(double *attendance, int *points, std::vector<arborTimetableEvent> *timetable, String *week) {
  if (connectWiFi()) {
    HTTPClient http;
    http.setConnectTimeout(3000); // 3 second max timeout
    http.begin(arborURL.c_str());
    http.addHeader("x-api-key", arborAPIKey);
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String payload = http.getString();
      JSONVar responseObject = JSON.parse(payload);
      if (JSON.typeof(responseObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      timetable->clear();
      *attendance = double(responseObject["attendance"]);
      *points = (int)responseObject["points"];
      *week = JSON.stringify(responseObject["week"]);
      for (int i = 0; i < 10; i++) {
        if (JSON.stringify(responseObject["timetable"][i]) == "" || JSON.stringify(responseObject["timetable"][i]) == "undefined" || JSON.stringify(responseObject["timetable"][i]) == "null") {
          Serial.println("Timetable event " + String(i) + " is empty!");
          break;
        }
        arborTimetableEvent event;
        event.eventStart = (int)responseObject["timetable"][i]["event_start"];
        event.eventEnd = (int)responseObject["timetable"][i]["event_end"];
        event.eventLocation = JSON.stringify(responseObject["timetable"][i]["location"]);
        event.eventLocation.substring(1, event.eventLocation.length() - 1); // Delete the quotes from the string
        event.eventEvent = JSON.stringify(responseObject["timetable"][i]["event"]);
        event.eventEvent.substring(1, event.eventEvent.length() - 1); // Delete the quotes from the string
        Serial.println("Event " + String(i) + ": " + event.eventEvent + " at " + event.eventLocation + " from " + String(event.eventStart) + " to " + String(event.eventEnd));
        (*timetable).push_back(event);
      }
    } else {
      // http error
    }
    http.end();
    // turn off radios
    WiFi.mode(WIFI_OFF);
    btStop();
  } else {
  }
}

// headblockheadSettings->sunRiseTime and headblockheadSettings->sunSetTime are ints of seconds since midnight
void WatchyHeadblockhead::getSunriseSunset(int *sunRiseTime, int *sunSetTime) {
  // Get OpenWeatherMap data
  if (connectWiFi()) {
    HTTPClient http;              // Use Weather API for live data if WiFi is connected
    http.setConnectTimeout(3000); // 3 second max timeout
    String weatherQueryURL = settings.weatherURL + settings.cityID + String("&units=") + settings.weatherUnit + String("&lang=") + settings.weatherLang + String("&appid=") + settings.weatherAPIKey;
    http.begin(weatherQueryURL.c_str());
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String payload = http.getString();
      JSONVar responseObject = JSON.parse(payload);
      if (JSON.typeof(responseObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      // Get sunrise/sunset times
      *sunRiseTime = ((int)responseObject["sys"]["sunrise"] + (int)responseObject["timezone"]) % 86400;
      ;
      *sunSetTime = ((int)responseObject["sys"]["sunset"] + (int)responseObject["timezone"]) % 86400;
      int gmtOffset = int(responseObject["timezone"]);
      syncNTP(gmtOffset);
    } else {
      // http error
    }
    http.end();
    // turn off radios
    WiFi.mode(WIFI_OFF);
    btStop();
  } else { // No WiFi, use clock for sunrise/sunset
    // If sunrise/sunset times are not set, use 7am/7pm
    if (headblockheadSettings->sunRiseTime == 0) {
      *sunRiseTime = 25200;
    }
    if (headblockheadSettings->sunSetTime == 0) {
      *sunSetTime = 72000; // 8pm
    }
  }
}

void WatchyHeadblockhead::drawSunriseSunset(int x, int y, int sunRiseTime, int sunSetTime) {
  char s[6];
  display.drawBitmap(x, y, sunrise, 20, 20, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.setFont(&DSEG7_Classic_Regular_15);
  display.setCursor(x + 25, y + 18);
  if (headblockheadSettings->is12Hrs) {
    sprintf(s, "%02d:%02d", ((sunRiseTime / 3600) % 12), (sunRiseTime / 60) % 60);
  } else {
    sprintf(s, "%02d:%02d", (sunRiseTime / 3600) % 24, (sunRiseTime / 60) % 60);
  }
  display.println(s);

  display.drawBitmap(x + 110, y, sunset, 20, 20, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.setCursor(x + 133, y + 18);
  if (headblockheadSettings->is12Hrs) {
    sprintf(s, "%02d:%02d", ((sunSetTime / 3600) % 12), (sunSetTime / 60) % 60);
  } else {
    sprintf(s, "%02d:%02d", (sunSetTime / 3600) % 24, (sunSetTime / 60) % 60);
  }
  display.println(s);
}

const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;
/*void WatchyHeadblockhead::drawWeather() {*/

/*weatherData currentWeather = getWeatherData();*/

/*int8_t temperature = currentWeather.temperature;*/
/*int16_t weatherConditionCode = currentWeather.weatherConditionCode;*/

/*display.setFont(&DSEG7_Classic_Regular_39);*/
/*int16_t x1, y1;*/
/*uint16_t w, h;*/
/*display.getTextBounds(String(temperature), 0, 0, &x1, &y1, &w, &h);*/
/*if (159 - w - x1 > 87) {*/
/*display.setCursor(159 - w - x1, 150);*/
/*} else {*/
/*display.setFont(&DSEG7_Classic_Bold_25);*/
/*display.getTextBounds(String(temperature), 0, 0, &x1, &y1, &w, &h);*/
/*display.setCursor(159 - w - x1, 136);*/
/*}*/
/*display.println(temperature);*/
/*display.drawBitmap(165, 110, currentWeather.isMetric ? celsius : fahrenheit, 26, 20, isDark ? GxEPD_WHITE : GxEPD_BLACK);*/
/*const unsigned char *weatherIcon;*/

/*// https://openweathermap.org/weather-conditions*/
/*if (weatherConditionCode > 801) { // Cloudy*/
/*weatherIcon = cloudy;*/
/*} else if (weatherConditionCode == 801) { // Few Clouds*/
/*weatherIcon = cloudsun;*/
/*} else if (weatherConditionCode == 800) { // Clear*/
/*weatherIcon = sunny;*/
/*} else if (weatherConditionCode >= 700) { // Atmosphere*/
/*weatherIcon = atmosphere;*/
/*} else if (weatherConditionCode >= 600) { // Snow*/
/*weatherIcon = snow;*/
/*} else if (weatherConditionCode >= 500) { // Rain*/
/*weatherIcon = rain;*/
/*} else if (weatherConditionCode >= 300) { // Drizzle*/
/*weatherIcon = drizzle;*/
/*} else if (weatherConditionCode >= 200) { // Thunderstorm*/
/*weatherIcon = thunderstorm;*/
/*} else*/
/*return;*/
/*display.drawBitmap(145, 158, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, isDark ? GxEPD_WHITE : GxEPD_BLACK);*/
/*}*/
