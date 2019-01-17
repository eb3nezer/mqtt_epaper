#ifndef _config_h
#define _config_h

#include <Ticker.h>
#include <PubSubClient.h>
#include <Timezone.h>

extern char mqtt_server[];
extern char mqtt_port[];
extern char mqtt_user[];
extern char mqtt_password[];
extern char mqtt_environment_topic[];
extern char mqtt_device[];
extern char time_server[];

extern PubSubClient mqttClient;

// Pins


// This is used to flash and report status.
#define CONFIG_PIN_STATUS BUILTIN_LED

extern Ticker ticker;
extern void tick();

// Enables Serial and print statements
#define CONFIG_DEBUG true

// Allow updates to firmware via wifi
#define ENABLE_OTA true
#define OTA_PORT 8266
#define OTA_PASSWORD "secret1"

extern Timezone auEastern;

#endif _config_h

