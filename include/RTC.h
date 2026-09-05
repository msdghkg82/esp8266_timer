#ifndef RTC_H_
#define RTC_H_


#pragma once

#include <sys/time.h>

#define TehranUTC_hour      3
#define TehranUTC_minute    30

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
void UpdateSysDate_systemTS();
void SetupSysTimestampIncrement();
void incrementSysTimestamp();
time_t get_RTCtimestamp();
void RTC_Begin();
void SysTS_RTC();


#endif /* RTC_H_ */