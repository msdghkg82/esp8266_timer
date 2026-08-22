#ifndef SAVEFILE_H
#define SAVEFILE_H

#pragma once

#include <LittleFS.h>

bool loadSchedulesFile();
bool saveSchedulesFile();
bool removeFile();

#endif /* SAVEFILE_H */