#ifndef _display_h
#define _display_h

#include <GxEPD2.h>
#include <GxEPD2_BW.h>

extern GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display;

extern void displayTime();
extern void displayWeather(int indoorTemp, int indoorHumidity, int outdoorTemp, int outdoorHumidity, int pressure);
extern void clearScreen();
extern void setupScreen();
extern void displayDialog(const char *message);
extern void clearDialog();
extern void displayStatus(bool internetConnected, bool mqttConnected, bool weatherRecent);
extern void displayConditions(const char *desc);

#endif _display_h
