#include <RTC.h>
#include <savefile.h>
#include <Ticker.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

volatile time_t systemtimestamp = 0;

volatile DateTime_ systemDate;

Ticker saveTimer;

void RTC_setDate(time_t timestamp)
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

void UpdateDate_RTC()
{
	DateTime dt = rtc.now();

	systemDate.year = dt.year();
	systemDate.month = dt.month();
	systemDate.day = dt.day();
	systemDate.hour = dt.hour();
	systemDate.minute = dt.minute();
	systemDate.second = dt.second();
}

void UpdateDate_systemTS()
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

void saveTimer_callback()
{
	saveTimestamp();
}

void startSavingTimestamp()
{
	saveTimer.attach(36000, saveTimer_callback);
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

void SetupSystemtimestampIncrement()
{
	timeval tv;
	tv.tv_sec = systemtimestamp;
	tv.tv_usec = 0;

	settimeofday(&tv, nullptr);
}

void incrementSystemtimestamp()
{
	systemtimestamp = time(nullptr);
}

time_t get_RTC_timestamp()
{
	DateTime dt = rtc.now();

	struct tm t{};
	t.tm_year = dt.year();
	t.tm_mon = dt.month();
	t.tm_mday = dt.day();
	t.tm_hour = dt.hour();
	t.tm_min = dt.minute();
	t.tm_sec = dt.second();

	return mktime(&t);
}