#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <GxEPD2.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_EPD.h>
#include <GxEPD2_GFX.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// I thought a 32 point font suited better. I generated this one using fontgen.
#include "calibri32ptb7b.h"
// Meteocons font by Alessio Atzeni
#include "WeatherIcon.h"

#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

#include "display.h"
#include "config.h"

const char TIME_FORMAT[] = "%02d:%02d";
const char DATE_FORMAT[] = "%02d/%02d/%04d";

void getTimeString(char *result) {
  time_t currentTime = auEastern.toLocal(now());
  sprintf(result, TIME_FORMAT,
      hour(currentTime),
      minute(currentTime));
}

void getDateString(char *result) {
  time_t currentTime = auEastern.toLocal(now());
  sprintf(result, DATE_FORMAT,
      day(currentTime),
      month(currentTime),
      year(currentTime));
}

void displayTime() {
  char buf[64];
  uint16_t x = 0;
  uint16_t y = 0;
  uint16_t w = 128;
  uint16_t h = 100;
  display.setPartialWindow(x, y, w, h);
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    getTimeString(buf);
    display.setFont(&FreeSansBold24pt7b);
    display.setCursor(x + 2, y + 36);
    display.println(buf);
    getDateString(buf);
    display.setFont(&FreeMono9pt7b);
    display.setCursor(x + 2, y + 70);
    display.println(buf);
  }
  while (display.nextPage());
}

void checkBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool checked) {
    display.drawLine(x, y, x + width, y, GxEPD_BLACK);
    display.drawLine(x + width, y, x + width, y + height, GxEPD_BLACK);
    display.drawLine(x + width, y + height, x, y + height, GxEPD_BLACK);
    display.drawLine(x, y + height, x, y, GxEPD_BLACK);
    if (checked) {
      display.drawLine(x, y, x + width, y + height, GxEPD_BLACK);
      display.drawLine(x + width, y, x, y + height, GxEPD_BLACK);
    }  
}

void setupScreenLines() {
  display.drawFastVLine(131, 0, 300, GxEPD_BLACK);
  display.drawFastHLine(0, 100, 131, GxEPD_BLACK);
  display.drawFastHLine(0, 200, 131, GxEPD_BLACK);
}

void displayConditions(const char *desc) {
  Serial.print("Weather ");
  Serial.println(desc);
  uint16_t x = 0;
  uint16_t y = 101;
  uint16_t w = 128;
  uint16_t h = 99;
  display.setPartialWindow(x, y, w, h);
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  display.setFont(&meteocons48pt7b);
  do
  {
    display.setCursor(x + 20, y + 85);
    if (!strcmp("clear-day", desc)) {
      display.print("B");
    } else if (!strcmp("sunny", desc)) {
      display.print("B");
    } else if (!strcmp("clear-night", desc)) {
      display.print("C");
    } else if (!strcmp("rain", desc)) {
      display.print("R");
    } else if (!strcmp("snow", desc)) {
      display.print("U");
    } else if (!strcmp("sleet", desc)) {
      display.print("P");
    } else if (!strcmp("wind", desc)) {
      display.print("F");
    } else if (!strcmp("windy", desc)) {
      display.print("F");
    } else if (!strcmp("fog", desc)) {
      display.print("M");
    } else if (!strcmp("cloudy", desc)) {
      display.print("N");
    } else if (!strcmp("partly-cloudy-day", desc)) {
      display.print("H");
    } else if (!strcmp("partlycloudy", desc)) {
      display.print("H");
    } else if (!strcmp("partly-cloudy-night", desc)) {
      display.print("I");
    } else {
      display.print(")"); // "NA"
    }
  }
  while (display.nextPage());
}

void displayStatus(bool internetConnected, bool mqttConnected, bool weatherRecent) {
  char buf[64];
  uint16_t x = 0;
  uint16_t y = 201;
  uint16_t w = 128;
  uint16_t h = 99;
  display.setPartialWindow(x, y, w, h);
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMono9pt7b);
    display.setCursor(x + 2, y + 17);
    display.println("Network");
    checkBox(x + 100, y + 7, 10, 10, internetConnected);
    display.setCursor(x + 2, y + 37);
    display.println("MQTT");
    checkBox(x + 100, y + 27, 10, 10, mqttConnected);
    display.setCursor(x + 2, y + 57);
    display.println("Weather");
    checkBox(x + 100, y + 47, 10, 10, weatherRecent);
  }
  while (display.nextPage());
}

void displayWeather(int indoorTemp, int indoorHumidity, int outdoorTemp, int outdoorHumidity, int pressure) {
  char buf[64];
  uint16_t x = 135;
  uint16_t y = 0;
  uint16_t w = 264;
  uint16_t h = 300;
  display.setPartialWindow(x, y, w, h);
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    // For some reason the fill screen overwrites part of the lines, even
    // though they are outside the partial window. Redraw them.
    setupScreenLines();
    display.setFont(&FreeMono9pt7b);
    display.setCursor(x + 4, y + 10);
    display.println("Inside");
    display.setCursor(x + 136, y + 10);
    display.println("Outside");
    //display.setFont(&FreeSansBold24pt7b);
    display.setFont(&calibrib32pt7b);
    display.setCursor(x + 4, y + 60);
    sprintf(buf, "%d.%01d", indoorTemp / 10, indoorTemp % 10);
    display.println(buf);
    display.setCursor(x + 4, y + 120);
    sprintf(buf, "%d%%", indoorHumidity / 10);
    display.println(buf);
    display.setCursor(x + 136, y + 60);
    sprintf(buf, "%d.%01d", outdoorTemp / 10, outdoorTemp % 10);
    display.println(buf);
    display.setCursor(x + 136, y + 120);
    sprintf(buf, "%d%%", outdoorHumidity / 10);
    display.println(buf);
    display.setCursor(x + 4, y + 155);
    display.setFont(&FreeMonoBold12pt7b);
    sprintf(buf, "%d.%01d hPa", pressure / 10, pressure % 10);
    display.println(buf);    
  }
  while (display.nextPage());
}

void clearScreen() {
  display.setRotation(0);
  display.setFullWindow();
  do
  {
    display.fillScreen(GxEPD_WHITE);
  }
  while (display.nextPage());  
}

void setupScreen() {
  display.setRotation(0);
  display.setFullWindow();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    setupScreenLines();
  }
  while (display.nextPage());  
}

void clearDialog() {
  uint16_t x = display.width() / 2 - 150;
  uint16_t y = display.height() / 2 - 50;
  uint16_t w = 300;
  uint16_t h = 100;
  display.setPartialWindow(x, y, w, h);
  display.setRotation(0);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  }
  while (display.nextPage());    
}

void displayDialog(const char *message) {
  uint16_t x = display.width() / 2 - 150;
  uint16_t y = display.height() / 2 - 50;
  uint16_t w = 300;
  uint16_t h = 100;
  display.setPartialWindow(x, y, w, h);
  display.setRotation(0);
  display.setFont(&FreeMono9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(x, y, w, 2, GxEPD_BLACK);
    display.fillRect(x, y + h - 2, w, 2, GxEPD_BLACK);
    display.setCursor(x, y + 15);
    display.println(message);    
  }
  while (display.nextPage());
}

