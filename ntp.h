#ifndef _ntp_h
#ifdef __cplusplus
#define _ntp_h

#include <Time.h>
#include <WiFiUdp.h>

#define NTP_PORT 2390

extern WiFiUDP Udp;

extern time_t requestNTPTime(void);

#endif
#endif
