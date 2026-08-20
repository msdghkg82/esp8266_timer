#include <RTC.h>

volatile time_t systemtimestamp = 0;

volatile DateTime Datetime;

void RTC_setTimestamp(time_t timestamp)
{
	timeval tv;
	tv.tv_sec = timestamp;
	tv.tv_usec = 0;

	settimeofday(&tv, nullptr);
}

void RTC_cc()
{
	systemtimestamp = time(nullptr);
}

void updateDate()
{
	time_t now = time(nullptr);
	struct tm * t = localtime(&now);
	Datetime.year = t->tm_year + 1900;
	Datetime.month = t->tm_mon + 1;
	Datetime.day = t->tm_mday;
	Datetime.hour = t->tm_hour;
	Datetime.minute = t->tm_min;
	Datetime.second = t->tm_sec;
}