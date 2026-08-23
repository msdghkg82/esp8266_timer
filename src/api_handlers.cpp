#include <api_handlers.h>

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <PersianDate.h>
#include <time.h>
#include <algorithm>

#include <RTC.h>
#include <Webserver.h>
#include <savefile.h>

#define TehranUTC 12600

Schedule_t ScheduleArray[50] = {0};

uint8_t ScheduleCount = 0;




uint8_t LEDstate = 1;

Ticker timer1;



PersianDate pd;




void handleRoot()
{
	server.send(200, "text/html", "<h1>Connected</h1>");
}

void handleStatus()
{
	JsonDocument json;
	json["SSID"] = WiFi.softAPSSID();
	json["IP"] = WiFi.softAPIP().toString();
	json["LED State"] = !(LEDstate) ? "high" : "low";

	String HandleRootJson;
	serializeJson(json, HandleRootJson);
	server.send(200, "application/json", HandleRootJson);
}


void timer1_callback()
{
	digitalWrite(LED_BUILTIN, !LEDstate);
}

void handleTimer()
{
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
	{
		server.send(200, "text/html", "<h1>invalid json</h1>");
		return;
	}

	if(doc["seconds"].is<float_t>())
	{
		float seconds = doc["seconds"].as<float_t>();
		if(doc["state"].is<uint8_t>())
		{
			LEDstate = doc["state"].as<uint8_t>();
			timer1.once(seconds, timer1_callback);
			server.send(200, "text/html", "<h1>تغییر وضعیت در " + String(seconds) + " ثانیه</h1>");
		}
		else
		{
			server.send(200, "text/html", "<h1>invalid state</h1>");
		}
	}
	else
	{
		server.send(200, "text/html", "<h1>invalid number of seconds</h1>");
	}
}



void handleTimestamp()
{
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
		{
			server.send(200, "text/html", "<h1>invalid json</h1>");
			return;
		}
	else
	{

		if(doc["timestamp"].is<uint32_t>())
		{
			systemtimestamp = doc["timestamp"].as<uint32_t>() + TehranUTC;
			RTC_setTimestamp(systemtimestamp);
			updateDate();
			server.send(200, "text/html", "<h1>timestamp set</h1>");
		}
	}
}

void handleDate()
{
	updateDate();
	pd.setGregorianDate(Datetime.year, Datetime.month, Datetime.day);
	pd.convertGregorianToPersian();

	JsonDocument doc;
	doc["Timestamp"] = systemtimestamp;

	doc["miladi"]["year"] = Datetime.year;
	doc["miladi"]["month"] = Datetime.month;
	doc["miladi"]["day"] = Datetime.day;
	doc["miladi"]["hour"] = Datetime.hour;
	doc["miladi"]["minute"] = Datetime.minute;
	doc["miladi"]["second"] = Datetime.second;

	doc["shamsi"]["date"] = pd.getFullPersianDateString();
	doc["shamsi"]["hour"] = Datetime.hour;
	doc["shamsi"]["minute"] = Datetime.minute;
	doc["shamsi"]["second"] = Datetime.second;

	String Datejson;
	serializeJson(doc, Datejson);
	server.send(200, "application/json", Datejson);
}

void sortSchedules()
{
	std::sort(ScheduleArray, ScheduleArray + ScheduleCount,
			[](const Schedule_t& a, const Schedule_t& b)
		{
			if(a.flag != b.flag)
			{
				return a.flag > b.flag;
			}
			return a.ScheduleTimeStamp < b.ScheduleTimeStamp;
		}
	);
}

uint8_t computeDayOfMonth(uint32_t timestamp)
{
	time_t t = (time_t)timestamp;
	struct tm ltm;
	localtime_r(&t,&ltm);
	Date shamsi = PersianDate::gregorianToPersian(
		ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday
	);
	return (uint8_t)shamsi.day;
}

void AddSchedule(uint32_t timestamp, uint8_t state, Repeat_t interval, uint16_t id)
{
	if(ScheduleCount < 50)
	{
		ScheduleArray[ScheduleCount].ScheduleTimeStamp = timestamp;
		ScheduleArray[ScheduleCount].state = state;
		ScheduleArray[ScheduleCount].interval = interval;
		if(interval == monthly)
		{
			ScheduleArray[ScheduleCount].dayOfMonth = computeDayOfMonth(timestamp);
		}
		ScheduleArray[ScheduleCount].id = id;
		ScheduleArray[ScheduleCount].flag = true;
		ScheduleCount++;
		sortSchedules();
		saveSchedulesFile();
	}
	else
	{
		server.send(200, "text/html", "<h1>no space</h1>");
	}
}

void handleSchedule()
{
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
		{
			server.send(200, "text/html", "<h1>invalid json</h1>");
			return;
		}
	else
	{
		if(doc["scheduletimestamp"].is<uint32_t>() && 
		   doc["state"].is<uint8_t>() && 
		   doc["interval"].is<uint8_t>() && 
		   doc["id"].is<uint16_t>()
		) {
			AddSchedule(doc["scheduletimestamp"], doc["state"], doc["interval"], doc["id"]);
			server.send(200, "text/html", "<h1>وظیفه تنظیم شد</h1>");
		}
		else
		{
			server.send(200, "text/html", "<h1>invalid keys</h1>");
		}
	}
}

/* void handleMonthlySchedules()
{
    time_t t = (time_t)ScheduleArray[0].ScheduleTimeStamp;
    struct tm ltm;
    localtime_r(&t, &ltm); 

    int wantedDay = ScheduleArray[0].dayOfMonth; 
    int hour      = ltm.tm_hour;
    int minute    = ltm.tm_min;

    Date jalaliNow = PersianDate::gregorianToPersian(
        ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday
    );

    int nextMonth = jalaliNow.month;
    int nextYear  = jalaliNow.year;
    uint32_t nextTs;

    nextMonth += 1;
    if (nextMonth > 12) {
        nextMonth = 1;
        nextYear += 1;
    }

    int dim;
    if (nextMonth <= 6) dim = 31;
    else if (nextMonth <= 11) dim = 30;
    else dim = PersianDate::isPersianLeapYear(nextYear) ? 30 : 29; 

    int actualDay = (wantedDay <= dim) ? wantedDay : dim; 

    Date greg = PersianDate::persianToGregorian(nextYear, nextMonth, actualDay);

    struct tm newTm = {0};
    newTm.tm_year  = greg.year - 1900;
    newTm.tm_mon   = greg.month - 1;
    newTm.tm_mday  = greg.day;
    newTm.tm_hour  = hour;
    newTm.tm_min   = minute;
    newTm.tm_sec   = 0;
    newTm.tm_isdst = -1;

    nextTs = (uint32_t)mktime(&newTm);

    ScheduleArray[0].ScheduleTimeStamp = nextTs;
    sortSchedules();
	saveSchedulesFile();
} */

void handleMonthlySchedules(Schedule_t& Schedule)
{
    struct tm miladi;
    time_t stamp = Schedule.ScheduleTimeStamp;

    // ساختن تقویم میلادی از تایم‌استمپ
    localtime_r(&stamp, &miladi);

    // تبدیل تقویم میلادی به شمسی
    Date shamsi = PersianDate::gregorianToPersian(miladi.tm_year, miladi.tm_mon, miladi.tm_mday);

    // روزی از ماه که تسک قراره روش اجرا بشه (اگر قبلاً ست نشده، از تاریخ فعلی می‌گیریم)
    uint8_t targetDay = Schedule.dayOfMonth ? Schedule.dayOfMonth : shamsi.day;
    Schedule.dayOfMonth = targetDay;

    // بردن ماه به جلو - همیشه، فارغ از تعداد روزهای ماه بعدی
    if (shamsi.month == 11)
    {
        shamsi.year += 1;
        shamsi.month = 0;
    }
    else
    {
        shamsi.month += 1;
    }

    // تعداد روزهای ماه جدید (بعد از پیشروی)
    int monthDays[12] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
    if (pd.isPersianLeapYear(shamsi.year)) monthDays[11] = 30;

    // اگر روز هدف توی ماه جدید وجود نداشت، به آخرین روز اون ماه محدودش کن
    uint8_t daysInNewMonth = monthDays[shamsi.month];
    shamsi.day = (targetDay <= daysInNewMonth) ? targetDay : daysInNewMonth;

    // تبدیل تاریخ شمسی جدید به میلادی
    Date tmp_miladi = PersianDate::persianToGregorian(shamsi.year, shamsi.month, shamsi.day);

    // ساختن تایم‌استمپ جدید (ساعت/دقیقه/ثانیه‌ی قبلی از localtime_r همچنان معتبره)
    miladi.tm_year = tmp_miladi.year;
    miladi.tm_mon  = tmp_miladi.month;
    miladi.tm_mday = tmp_miladi.day;

    Schedule.ScheduleTimeStamp = (uint32_t)mktime(&miladi);

    sortSchedules();
    saveSchedulesFile();
}

void ProcessSchedules()
{
	if(ScheduleCount == 0)
		return;

	if(ScheduleArray[0].flag == false)
		return;

	if(ScheduleArray[0].ScheduleTimeStamp > systemtimestamp)
		return;

	digitalWrite(LED_BUILTIN, !ScheduleArray[0].state);

	if(ScheduleArray[0].interval == once)
	{
		ScheduleArray[0].flag = false;
		sortSchedules();
		ScheduleCount--;
		saveSchedulesFile();
	}
	else if(ScheduleArray[0].interval == daily)
	{
		ScheduleArray[0].ScheduleTimeStamp += 86400;
		//ScheduleArray[i].flag = true;
		sortSchedules();
		saveSchedulesFile();
	}
	else if(ScheduleArray[0].interval == weekly)
	{
		ScheduleArray[0].ScheduleTimeStamp += 604800;
		//ScheduleArray[i].flag = true;
		sortSchedules();
		saveSchedulesFile();
	}
	else if(ScheduleArray[0].interval == monthly)
	{
		handleMonthlySchedules(ScheduleArray[0]);
	}
}

void handleGetSchedules()
{
    JsonDocument doc;

	doc["SystemTimestamp"] = systemtimestamp;
	doc["ScheduleCount"] = ScheduleCount;

    JsonArray array = doc["schedules"].to<JsonArray>();

    for (uint8_t i = 0; i < 50; i++) {

        JsonObject obj = array.add<JsonObject>();

		obj["id"]   	 = ScheduleArray[i].id;
        obj["timestamp"] = ScheduleArray[i].ScheduleTimeStamp;
		obj["state"]     = ScheduleArray[i].state;
        obj["interval"]  = ScheduleArray[i].interval;
		if(ScheduleArray[i].interval == monthly)
		{
			obj["dayOfMonth"] = ScheduleArray[i].dayOfMonth;
		}
        obj["enabled"]   = ScheduleArray[i].flag;
    }

    String response;
    serializeJson(doc, response);

    server.send(200, "application/json", response);
}

void handleResetSchedules()
{
	for(uint8_t i = 0; i < ScheduleCount; i++)
	{
		ScheduleArray[i] = Schedule_t{};
	}
	ScheduleCount = 0;
	//memset(ScheduleArray, 0, sizeof(ScheduleArray));
	server.send(200, "text/html", "<h1>schedules reset</h1>");
	//removeFile();
}

