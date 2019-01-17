#include <FS.h> // Make sure this is first

#include <Arduino.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <elapsedMillis.h>

#include "config.h"
#include "display.h"

#define WIFI_RECONNECT_POLLING_PERIOD 5000

bool shouldSaveConfig = false;

elapsedMillis wifiReconnectTimeElapsed;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void readConfigFromFilesystem() {
  //read configuration from FS json

  //Serial.println("Formatting filesystem...");
  //SPIFFS.format();
    
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("parsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_password, json["mqtt_password"]);
          if (json.containsKey("mqtt_environment_topic")) {
            strcpy(mqtt_environment_topic, json["mqtt_environment_topic"]);            
          }
          strcpy(mqtt_device, json["mqtt_device"]);
          strcpy(time_server, json["time_server"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end 
}

void writeConfigToFilesystem() {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_password"] = mqtt_password;
    json["mqtt_environment_topic"] = mqtt_environment_topic;
    json["mqtt_device"] = mqtt_device;
    json["time_server"] = time_server;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
}

void configureWifi(bool autoConfig) {
  ticker.attach(0.2, tick);

  readConfigFromFilesystem();
  
  shouldSaveConfig = false;
  
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "MQTT server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "MQTT user", mqtt_user, 63);
  WiFiManagerParameter custom_mqtt_password("pass", "MQTT password", mqtt_password, 63);
  WiFiManagerParameter custom_mqtt_environment_topic("mqtt_environment_topic", "Envir. MQTT Topic", mqtt_environment_topic, 63);
  WiFiManagerParameter custom_mqtt_device("device", "MQTT device", mqtt_device, 63);
  WiFiManagerParameter custom_time_server("timeserver", "NTP Server", time_server, 63);
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  //set minimu quality of signal so it ignores APs under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality();

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
  wifiManager.addParameter(&custom_mqtt_environment_topic);
  wifiManager.addParameter(&custom_mqtt_device);
  wifiManager.addParameter(&custom_time_server);

  String apName = "ePaper";
  apName += WiFi.softAPmacAddress();
  apName.replace(":", "");
  // Display the access point name
  displayDialog(apName.c_str());
  if (autoConfig) {
    Serial.println("Auto connect to wifi");
    if (!wifiManager.autoConnect(apName.c_str())) {
      Serial.println("Auto connect failed");
      delay(3000);
      // if autoConfig is true, then we are at startup, so reset and try again
      ESP.reset();
      delay(5000);
    }
  } else {
    Serial.println("Reconfiguring...");
    if (!wifiManager.startConfigPortal(apName.c_str())) {
      Serial.println("Reconfigure failed to connect to wifi");
      shouldSaveConfig = false;
    }
  }

  ticker.detach();
  digitalWrite(CONFIG_PIN_STATUS, LOW);
  
  if (shouldSaveConfig) {
    //read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());
    strcpy(mqtt_environment_topic, custom_mqtt_environment_topic.getValue());
    strcpy(mqtt_device, custom_mqtt_device.getValue());
    strcpy(time_server, custom_time_server.getValue());
  
    Serial.print("MQTT Server: ");
    Serial.println(mqtt_server);
    Serial.print("MQTT Port: ");
    Serial.println(mqtt_port);
    Serial.print("MQTT User: ");
    Serial.println(mqtt_user);
    Serial.print("MQTT Password: ");
    Serial.println(mqtt_password);
    Serial.print("MQTT environment topic: ");
    Serial.println(mqtt_environment_topic);
    Serial.print("MQTT Device: ");
    Serial.println(mqtt_device);
    Serial.print("Time Server: ");
    Serial.println(time_server);

    writeConfigToFilesystem();
  } else {
    Serial.println("Config was not updated");
  }

  clearDialog();
}

void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiReconnectTimeElapsed > WIFI_RECONNECT_POLLING_PERIOD) {
      wifiReconnectTimeElapsed = 0;
      Serial.print("Reconnecting to WiFi...");
      ticker.attach(0.05, tick);
      WiFi.begin();
      ticker.detach();
      digitalWrite(CONFIG_PIN_STATUS, LOW);
      if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Failed to connect to WiFi");        
      }
    }
  }
}
