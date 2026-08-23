#ifndef API_HANDLERS_H_
#define API_HANDLERS_H_


#pragma once

#include <stdint.h>

typedef enum {
	once = 0,
	daily,
	weekly,
	monthly
} Repeat_t;

typedef struct {
	uint16_t id;
	uint32_t ScheduleTimeStamp;
	uint8_t state;
	Repeat_t interval;
	uint8_t dayOfMonth = 0;
	bool flag = false;
} Schedule_t;

extern Schedule_t ScheduleArray[50];

extern uint8_t ScheduleCount;

void handleRoot();
void handleStatus();
void handleTimer();
void handleTimestamp();
void handleDate();
void handleSchedule();
void ProcessSchedules();
void handleGetSchedules();
void handleResetSchedules();
void handleRemoveSchedule();

#endif /* API_HANDLERS_H_ */