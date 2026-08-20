#ifndef API_HANDLERS_H_
#define API_HANDLERS_H_


#pragma once

#include <stdint.h>

struct Schedule {
	uint16_t id;
	uint32_t ScheduleTimeStamp;
	uint8_t state;
	bool flag = false;
};

extern Schedule ScheduleArray[50];

extern uint8_t ScheduleCount;

void handleRoot();
void handleStatus();
void handleTimer();
void handleTimestamp();
void handleDate();
void handleSchedule();
void ProcessSchedules();
void handleGetSchedules();


#endif /* API_HANDLERS_H_ */