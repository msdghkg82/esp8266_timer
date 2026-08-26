#ifndef RTC_H_
#define RTC_H_


#pragma once

#include <sys/time.h>

extern volatile time_t systemtimestamp;

typedef struct {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
} DateTime_t;

extern volatile DateTime_t systemDate;

void RTC_SetTimestamp(time_t timestamp);
//void UpdateSysDate_RTC();
void UpdateSysDate_systemTS();
void SetupSysTimestampIncrement();
void incrementSysTimestamp();
time_t get_RTC_timestamp();
void RTC_Begin();


#endif /* RTC_H_ */