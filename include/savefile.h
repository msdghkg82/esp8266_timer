#ifndef SAVEFILE_H
#define SAVEFILE_H

#pragma once

#include <LittleFS.h>

#define SCHEDULE_FILE    "/schedules.json"
#define WIFICONFIG_FILE  "/wificonfig.json"
#define LOGLANG_FILE     "/loglang.json"

void mountLittleFS();
void LoadFiles();
bool loadSchedulesFile();
bool saveSchedulesFile();
bool saveWifiConfig();
bool loadWifiConfig();
bool saveLogLang();
bool loadLogLang();
bool removeFile(String path);

#endif /* SAVEFILE_H */