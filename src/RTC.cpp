#include <RTC.h>
#include <savefile.h>
#include <Ticker.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

volatile time_t systemtimestamp = 0;

volatile DateTime_t systemDate;

void RTC_SetTimestamp(time_t timestamp)
{
	struct tm t{};
	time_t now = timestamp;
	localtime_r(&now, &t);

	DateTime dt(
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_mday,
		t.tm_hour,
		t.tm_min,
		t.tm_sec
	);

	rtc.adjust(dt);
}

void UpdateSysDate_systemTS()
{
	time_t now = systemtimestamp;
	struct tm t{}; 
	localtime_r(&now, &t);
	systemDate.year = t.tm_year + 1900;
	systemDate.month = t.tm_mon + 1;
	systemDate.day = t.tm_mday;
	systemDate.hour = t.tm_hour;
	systemDate.minute = t.tm_min;
	systemDate.second = t.tm_sec;
}

void RTC_Begin()
{
	Wire.begin();
	if(!rtc.begin())
	{
		Serial.println("RTC not found");
		while(1);
	}
	else
	{
		Serial.println("RTC connected");
	}
}

void SetupSysTimestampIncrement()
{
	timeval tv;
	tv.tv_sec = systemtimestamp;
	tv.tv_usec = 0;

	settimeofday(&tv, nullptr);
}

void incrementSysTimestamp()
{
	systemtimestamp = time(nullptr);
}

time_t get_RTCtimestamp()
{
	DateTime dt = rtc.now();

	struct tm t{};
	t.tm_year = dt.year() - 1900;
	t.tm_mon = dt.month() - 1;
	t.tm_mday = dt.day();
	t.tm_hour = dt.hour();
	t.tm_min = dt.minute();
	t.tm_sec = dt.second();

	return mktime(&t);
}

void SysTS_RTC()
{
	systemtimestamp = get_RTCtimestamp();
	SetupSysTimestampIncrement();
}