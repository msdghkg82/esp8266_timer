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

void RemoveSchedule(uint16_t id)
{
	for(uint8_t i = 0; i < ScheduleCount; i++)
	{
		if(ScheduleArray[i].id == id)
		{
			ScheduleArray[i].id = 0;
			ScheduleArray[i].flag = false;
			sortSchedules();
			ScheduleCount--;
			saveSchedulesFile();
			break;
		}
	}
}

void handleRemoveSchedule()
{
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
		{
			server.send(200, "text/html", "<h1>invalid id</h1>");
			return;
		}
	else
	{
		if(doc["id"].is<uint16_t>())
		{
			RemoveSchedule(doc["id"]);
			server.send(200, "text/html", "<h1>schedule removed</h1>");
		}
	}
}

void AddSchedule(uint32_t timestamp, uint8_t state, Repeat_t interval, uint16_t id)
{
	if(ScheduleCount < 50)
	{
		ScheduleArray[ScheduleCount].ScheduleTimeStamp = timestamp;
		ScheduleArray[ScheduleCount].state = state;
		ScheduleArray[ScheduleCount].interval = interval;
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



void handleMonthlySchedules(Schedule_t& Schedule)
{
	struct tm miladi{};
	time_t timestamp = (time_t)Schedule.ScheduleTimeStamp;
	localtime_r(&timestamp, &miladi);

	Serial.println("tarikh miladi ghabl az taghir");
	Serial.println(String("/") + miladi.tm_year + String("/") + miladi.tm_mon + String("/") + miladi.tm_mday);

	Date shamsi = PersianDate::gregorianToPersian(miladi.tm_year + 1900, miladi.tm_mon + 1, miladi.tm_mday);

	Serial.println("tarikh ghabl az taghir");
	Serial.println(String("/") + shamsi.year + String("/") + shamsi.month + String("/") + shamsi.day);

	int monthDays[12] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};

	if(Schedule.dayOfMonth == 0)
	{
		Schedule.dayOfMonth = shamsi.day;
		Serial.println("dayOfMonth set to " + String(Schedule.dayOfMonth));
	}

	while (true)
	{		
		if(shamsi.month == 12)
		{
			shamsi.year++;
			shamsi.month = 1;
		}
		else
		{
			shamsi.month++;
		}

		Serial.println("tarikh ba'd az taghir");
		Serial.println(String("/") + shamsi.year + String("/") + shamsi.month + String("/") + shamsi.day);

  		if(PersianDate::isPersianLeapYear(shamsi.year))
		{
			monthDays[11] = 30;
		}

		Serial.println(String("isPersianLeapYear: ") + String(PersianDate::isPersianLeapYear(shamsi.year)));
		Serial.println("roozaye esfand");
		Serial.println(monthDays[shamsi.month - 1]);

		if(Schedule.dayOfMonth <= monthDays[shamsi.month - 1])
			break;
	}

	Date TMPmiladi = PersianDate::persianToGregorian(shamsi.year, shamsi.month, Schedule.dayOfMonth);

	Serial.println("tarikh miladi ghabl az taghir");
	Serial.println(String("/") + TMPmiladi.year + String("/") + TMPmiladi.month + String("/") + TMPmiladi.day);

	miladi.tm_year = TMPmiladi.year - 1900;
	miladi.tm_mon = TMPmiladi.month - 1;
	miladi.tm_mday = TMPmiladi.day;

	Serial.println("tarikh miladi ba'd az taghir");
	Serial.println(String("/") + miladi.tm_year + String("/") + miladi.tm_mon + String("/") + miladi.tm_mday);

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
	server.send(200, "text/html", "<h1>schedules reset</h1>");
	//removeFile();
	saveSchedulesFile();
}

