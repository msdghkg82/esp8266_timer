#ifndef API_HANDLERS_H_
#define API_HANDLERS_H_


#pragma once

#include <stdint.h>
#include <time.h>

typedef enum {
	once = 0,
	daily,
	weekly,
	monthly
} Repeat_t;

typedef struct {
	uint16_t id;
	time_t ScheduleTimeStamp;
	uint8_t state;
	Repeat_t interval;
	bool flag = false;
} Schedule_t;

extern Schedule_t ScheduleArray[50];

extern uint8_t ScheduleCount;

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
void ProcessSchedules();


#endif /* API_HANDLERS_H_ */