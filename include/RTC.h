#ifndef RTC_H_
#define RTC_H_


#pragma once

#include <sys/time.h>

extern volatile time_t systemtimestamp;

struct DateTime {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};

extern volatile DateTime Datetime;

void RTC_setTimestamp(time_t timestamp);
void RTC_cc();
void updateDate();
void startSavingTimestamp();


#endif /* RTC_H_ */