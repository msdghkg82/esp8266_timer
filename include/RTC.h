#ifndef RTC_H_
#define RTC_H_


#pragma once

#include <sys/time.h>

extern volatile time_t systemtimestamp;

struct DateTime_ {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};

extern volatile DateTime_ systemDate;

void RTC_setDate(time_t timestamp);
void UpdateDate_RTC();
void UpdateDate_systemTS();
void SetupSystemtimestampIncrement();
void incrementSystemtimestamp();
time_t get_RTC_timestamp();
void startSavingTimestamp();
void RTC_Begin();


#endif /* RTC_H_ */