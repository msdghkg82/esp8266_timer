#ifndef SAVEFILE_H
#define SAVEFILE_H

#pragma once

#include <LittleFS.h>

#define SCHEDULE_FILE    "/schedules.json"
#define WIFICONFIG_FILE  "/wificonfig.json"

void mountFile();
bool loadSchedulesFile();
bool saveSchedulesFile();
bool saveWifiConfig();
bool loadWifiConfig();
bool removeFile(String path);

#endif /* SAVEFILE_H */