#ifndef SAVEFILE_H
#define SAVEFILE_H

#pragma once

#include <LittleFS.h>

bool loadSchedulesFile();
bool saveSchedulesFile();
bool removeFile(String path);
bool saveTimestamp();
bool loadTimestamp();

#endif /* SAVEFILE_H */