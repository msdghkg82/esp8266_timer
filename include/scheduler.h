#ifndef SCHEDULER_H_
#define SCHEDULER_H_


#pragma once

#include <stdint.h>
#include <time.h>

#define RELAY_PIN 14

typedef enum {
	once = 0,
	daily,
	weekly,
	monthly
} Repeat_t;

typedef struct {
	uint8_t id;
	time_t ScheduleTimeStamp;
	uint8_t state;
	Repeat_t interval;
	bool flag = false;
} Schedule_t;

extern Schedule_t ScheduleArray[50];
extern uint8_t ScheduleCount;

bool RemoveSchedule(uint8_t id);
bool AddSchedule(time_t timestamp, uint8_t state, Repeat_t interval, uint8_t id);
bool ResetSchedules();
void ProcessSchedules();
void Scheduler_Init();


#endif /* SCHEDULER_H_ */