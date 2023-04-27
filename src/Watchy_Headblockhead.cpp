#include "Watchy_Headblockhead.h"

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

bool isDark = false;

void WatchyHeadblockhead::handleButtonPress() {
  if (guiState != WATCHFACE_STATE) { // If we are not on the watchface, don't use custom button handling.
    Watchy::handleButtonPress();
    return;
  }
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
  if (wakeupBit & MENU_BTN_MASK) {
    Watchy::handleButtonPress(); // Use default button handling for menu button.
    return;
  }
  RTC.read(currentTime); // Update the current time.
  if (wakeupBit & UP_BTN_MASK) {
    return;
  }
  if (wakeupBit & DOWN_BTN_MASK) {
    return;
  }
  if (wakeupBit & BACK_BTN_MASK) {
    return;
  }
  showWatchFace(true); // True for partial refresh.
  return;
}

void WatchyHeadblockhead::drawWatchFace() {
  display.fillScreen(isDark ? GxEPD_BLACK : GxEPD_WHITE);
  display.setTextColor(isDark ? GxEPD_WHITE : GxEPD_BLACK);
  // Screen bounds: 200x200

  // Top statusbar: 200x25
  drawBattery(2, 2);    // 40x20
  drawWifi(43, 4);      // 26x18
  drawBluetooth(72, 2); // 13x20
  display.drawLine(0, 25, 85, 25, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.drawLine(85, 25, 93, 0, isDark ? GxEPD_WHITE : GxEPD_BLACK);

  // Top Right: 118x25
  drawSteps(100, 4, sensor.getCounter()); // 116x20

  // Main time.
  drawTime(5, 85);   // 200x100
  drawDate(39, 100); // 118x25
}
void WatchyHeadblockhead::drawBattery(int x, int y) {
  display.drawBitmap(x, y, battery, 37, 21, isDark ? GxEPD_WHITE : GxEPD_BLACK);
  display.fillRect(x + 5, y + 5, 27, BATTERY_SEGMENT_HEIGHT, isDark ? GxEPD_BLACK : GxEPD_WHITE); // clear battery segments
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
    display.fillRect(x + 5 + (batterySegments * BATTERY_SEGMENT_SPACING), y + 5, BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT, isDark ? GxEPD_WHITE : GxEPD_BLACK);
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
  if (HOUR_12_24 == 12) {
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
