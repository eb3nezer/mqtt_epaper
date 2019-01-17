#include <ArduinoJson.h>
#include <elapsedMillis.h>

#include "config.h"
#include "mqtt.h"
#include "display.h"

// If disconnected, how often to try to reconnect
#define RECONNECT_POLLING_PERIOD 5000

elapsedMillis reconnectTimeElapsed;
elapsedMillis timeSinceLastMessage;

int outdoorTemp = 0;
int indoorTemp = 0;
int outdoorHumidity = 0;
int indoorHumidity = 0;
int pressure = 0;
char weatherDesc[32] = "";
char oldWeatherDesc[32] = "";

int oldOutdoorTemp = 0;
int oldIndoorTemp = 0;
int oldOutdoorHumidity = 0;
int oldIndoorHumidity = 0;
int oldPressure = 0;

bool processJson(char* message) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }
  
  if (root.containsKey("outdoor_temp")) {
    sscanf(root["outdoor_temp"], "%d", &outdoorTemp);
  }
  if (root.containsKey("indoor_temp")) {
    sscanf(root["indoor_temp"], "%d", &indoorTemp);
  }
  if (root.containsKey("outdoor_hum")) {
    sscanf(root["outdoor_hum"], "%d", &outdoorHumidity);
  }
  if (root.containsKey("indoor_hum")) {
    sscanf(root["indoor_hum"], "%d", &indoorHumidity);
  }
  if (root.containsKey("pressure")) {
    sscanf(root["pressure"], "%d", &pressure);
  }

  Serial.println("Parsed MQTT message");

  if ((indoorTemp != oldIndoorTemp) ||
      (indoorHumidity != oldIndoorHumidity) ||
      (outdoorTemp != oldOutdoorTemp) ||
      (outdoorHumidity != oldOutdoorHumidity) ||
      (oldPressure != pressure)) {
    Serial.println("Weather changed. Updating screen.");
    // TODO do a partial update of each
    displayWeather(indoorTemp, indoorHumidity, outdoorTemp, outdoorHumidity, pressure);        
  }

  if (root.containsKey("weather")) {
    strcpy(weatherDesc, root["weather"]);
    if (strlen(weatherDesc) > 0 && strcmp(oldWeatherDesc, weatherDesc)) {
      Serial.println("Conditions changed. Updating screen.");
      displayConditions(weatherDesc);
    } else {
      Serial.println("Weather conditions did not change.");
    }
  }

  oldIndoorTemp = indoorTemp;
  oldIndoorHumidity = indoorHumidity;
  oldOutdoorTemp = outdoorTemp;
  oldOutdoorHumidity = outdoorHumidity;
  oldPressure = pressure;
  strcpy(oldWeatherDesc, weatherDesc);
  
  return true;
}

void processMessage(char* topic, byte* payload, unsigned int length) {
  timeSinceLastMessage = 0;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!processJson(message)) {
    return;
  }
}

void reconnectMqtt() {
  if (!mqttClient.connected()) {
    if (reconnectTimeElapsed > RECONNECT_POLLING_PERIOD) {
      reconnectTimeElapsed = 0;
      Serial.print("Attempting MQTT connection...");
      ticker.attach(0.05, tick);
      // Attempt to connect
      if (mqttClient.connect(mqtt_device, mqtt_user, mqtt_password)) {
        Serial.println("connected");
        if (!strlen(mqtt_environment_topic)) {
          Serial.println("Error: mqtt_environment_topic not set");
        } else {
          Serial.print("Subscribe to: ");
          Serial.println(mqtt_environment_topic);
          mqttClient.subscribe(mqtt_environment_topic);          
        }
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
      }
      ticker.detach();
      digitalWrite(CONFIG_PIN_STATUS, LOW);
    }
  }
}

bool weatherUpdatedRecently() {
  return (timeSinceLastMessage < (10 * 60 * 1000));
}

