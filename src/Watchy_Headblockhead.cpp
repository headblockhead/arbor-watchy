#include "Watchy_Headblockhead.h"

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
    Watchy::handleButtonPress();
    return;
  } else if (wakeupBit & UP_BTN_MASK) {
  } else if (wakeupBit & DOWN_BTN_MASK) {
    vibMotor(10, 10);
    headblockheadSettings->is12Hrs = !headblockheadSettings->is12Hrs;
  } else if (wakeupBit & BACK_BTN_MASK) {
  }
  drawWatchFace();
  Watchy::showWatchFace(true);
  return;
}

bool isDark = false;

void WatchyHeadblockhead::drawWatchFace() {
  // If after sunset, switch to night mode.

  if ((currentTime.Hour == 0 && currentTime.Minute == 1) || headblockheadSettings->sunRiseTime == 0 || headblockheadSettings->sunSetTime == 0) {
    getSunriseSunset(&headblockheadSettings->sunRiseTime, &headblockheadSettings->sunSetTime);
  }

  isDark = ((currentTime.Second) + (60 * currentTime.Minute) + (3600 * currentTime.Hour)) < headblockheadSettings->sunRiseTime || ((currentTime.Second) + (60 * currentTime.Minute) + (3600 * currentTime.Hour)) > headblockheadSettings->sunSetTime;

  // Set the colors.
  display.fillScreen(isDark ? GxEPD_BLACK : GxEPD_WHITE);
  display.setTextColor(isDark ? GxEPD_WHITE : GxEPD_BLACK);

  // Screen bounds: 200x200

  // Top Left widget:
  drawBattery(2, 5);    // 40x20
  drawWifi(42, 6);      // 26x18
  drawBluetooth(72, 5); // 13x20
  // Bottom line
  display.drawLine(0, 30, 90, 30, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  // Right line
  display.drawLine(90, 30, 98, 0, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  // Top Right widget:
  drawSteps(111, 6, sensor.getCounter());
  // Bottom line
  display.drawLine(110, 30, 200, 30, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  // Left line
  display.drawLine(102, 0, 110, 30, isDark ? GxEPD_WHITE : GxEPD_BLACK);

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

  // Right:
  // Top line
  display.drawLine(110, 170, 200, 170, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  // Left line
  display.drawLine(102, 200, 110, 170, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  return;
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
      *sunRiseTime = (int)responseObject["sys"]["sunrise"] + (int)responseObject["timezone"];
      *sunSetTime = (int)responseObject["sys"]["sunset"] + (int)responseObject["timezone"];
      int gmtOffset = int(responseObject["timezone"]);
      syncNTP(gmtOffset);
    } else {
      // http error
    }
    http.end();
    // turn off radios
    WiFi.mode(WIFI_OFF);
    btStop();
  } else {                // No WiFi, use clock for sunrise/sunset
    *sunRiseTime = 25200; // 7am
    *sunSetTime = 72000;  // 8pm
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
