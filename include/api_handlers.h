#ifndef API_HANDLERS_H_
#define API_HANDLERS_H_

#pragma once
#include <stdint.h>

extern uint8_t RelayState;

void handle_Root();
void handle_GetStatus();
void handle_SetTimer();
void handle_SetSysTimestamp();
void handle_GetDate();
void handle_SetSchedule();
void handle_GetSchedules();
void handle_ResetSchedules();
void handle_RemoveSchedule();
void handle_GetSavedFile();
void handle_wifisetting();
void handle_wifioff();

#endif /* API_HANDLERS_H_ */