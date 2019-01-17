#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#define ENABLE_GxEPD2_GFX 0
#define ENABLE_OTA 1

#include <GxEPD2.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_EPD.h>
#include <GxEPD2_GFX.h>

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Ticker.h>
#include <elapsedMillis.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <elapsedMillis.h>

#include "config.h"
#include "ntp.h"
#include "configure_wifi.h"
#include "mqtt.h"
#include "display.h"

// Waveshare 4.2" b&W display
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=15*/ SS, /*DC=4*/ 4, /*RST=5*/ 5, /*BUSY=16*/ 16));

Ticker ticker;

char mqtt_server[40] = "";
char mqtt_port[6] = "1883";
char mqtt_user[64] = "";
char mqtt_password[64] = "";
char mqtt_environment_topic[64] = "";
char mqtt_device[64] = "";
char time_server[64] = "";

bool prevWifiStatus;
bool prevMQTTStatus;
bool prevWeatherUpdateStatus;

WiFiClient wifiClient;
WiFiUDP Udp;
PubSubClient mqttClient(wifiClient);

time_t lastUpdate = 0;
uint16_t lastMinute = -1;

// Australia NSW
TimeChangeRule auEDT = {"EDT", First, Sun, Oct, 2, 660};  //UTC + 11 hours
TimeChangeRule auEST = {"EST", First, Sun, Apr, 2, 600};   //UTC + 10 hours
Timezone auEastern(auEDT, auEST);

void tick() {
  //toggle state
  int state = digitalRead(CONFIG_PIN_STATUS);  // get the current state of GPIO1 pin
  digitalWrite(CONFIG_PIN_STATUS, !state);     // set pin to the opposite state
}

void setup() {
  Serial.begin(115200);

  //set led pin as output
  pinMode(CONFIG_PIN_STATUS, OUTPUT);
  digitalWrite(CONFIG_PIN_STATUS, LOW); // Turn off the on-board LED
  
  //use flash button as input
  pinMode(D3, INPUT);

  display.init(115200);
  clearScreen();
  configureWifi(true);

#ifdef ENABLE_OTA
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
#endif

  Udp.begin(NTP_PORT);

  digitalWrite(CONFIG_PIN_STATUS, LOW); // Turn off the on-board LED

  setupScreen();

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(processMessage);
  
  Serial.println();
  Serial.println("setup");

  // Sync time once per couple of hours
  setSyncInterval(7300);
  setSyncProvider(requestNTPTime);

  prevWifiStatus = (WiFi.status() == WL_CONNECTED);
  prevMQTTStatus = mqttClient.connected();
  prevWeatherUpdateStatus = weatherUpdatedRecently();
  displayStatus(prevWifiStatus, prevMQTTStatus, prevWeatherUpdateStatus);
  Serial.print("Wifi status is ");
  Serial.println(WiFi.status());
}

void loop() {
  // Check for user pressing the onboard NodeMCU/ESP-12E "FLASH" button, which means reconfigure
  if (digitalRead(D3) == LOW) {
    configureWifi(false);
  }

  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  } else {
    if (!mqttClient.connected()) {
      reconnectMqtt();
    }
    if (mqttClient.connected()) {
      mqttClient.loop();    
    }    
  }
  
  time_t currentTime = now();
  uint16_t currentMinute = minute(currentTime);
  if (currentMinute != lastMinute) {
    Serial.println("Update time");
    displayTime();
    lastUpdate = currentTime;
    lastMinute = currentMinute;
  }

  if ((prevWifiStatus != (WiFi.status() == WL_CONNECTED)) ||
   (prevMQTTStatus != mqttClient.connected()) ||
   (prevWeatherUpdateStatus != weatherUpdatedRecently())) {
    prevWifiStatus = WiFi.status() == WL_CONNECTED;
    prevMQTTStatus = mqttClient.connected();
    prevWeatherUpdateStatus = weatherUpdatedRecently();
    displayStatus(prevWifiStatus, prevMQTTStatus, prevWeatherUpdateStatus);
   }
}
