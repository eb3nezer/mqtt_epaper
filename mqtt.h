#ifndef _mqtt_h
#define _mqtt_h

extern void processMessage(char* topic, byte* payload, unsigned int length);
extern void reconnectMqtt();
extern bool weatherUpdatedRecently();

#endif _mqtt_h
