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
			server.send(200, "text/html", "<h1>تغییر وضعیت در 3 ثانیه</h1>");
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

void ProcessSchedules()
{
	if(ScheduleArray[0].flag == false)
	return;

	if(ScheduleArray[0].ScheduleTimeStamp > systemtimestamp)
	return;

	digitalWrite(LED_BUILTIN, !ScheduleArray[0].state);

	if(ScheduleArray[0].interval == once)
	{
		ScheduleArray[0].flag = false;
		ScheduleCount--;
		sortSchedules();
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
		handleMonthlySchedules();
	}
}

void handleMonthlySchedules()
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
}

void handleGetSchedules()
{
    JsonDocument doc;

    JsonArray array = doc["schedules"].to<JsonArray>();

    for (uint8_t i = 0; i < ScheduleCount; i++) {

        JsonObject obj = array.add<JsonObject>();

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

