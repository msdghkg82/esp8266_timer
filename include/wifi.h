#ifndef WIFI_H_
#define WIFI_H

#pragma once

#include <WString.h>

extern String SSID;
extern String PASSWD;

void wifi_setup();
void wifi_off();

#endif /* WIFI_H */