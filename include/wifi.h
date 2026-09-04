#ifndef WIFI_H
#define WIFI_H

#pragma once

#include <WString.h>

extern String AP_SSID;
extern String AP_PASSWD;
extern String STA_SSID;
extern String STA_PASSWD;

void wifi_off();
void wifi_on();
void wifi_toggle_onoff();
void wifi_toggle_mode();

#endif /* WIFI_H */