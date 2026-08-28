#include <api_handlers.h>

/* Built-in Libraries */
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <PersianDate.h>
#include <time.h>
#include <algorithm>
/**********************/

#include <RTC.h>
#include <Webserver.h>
#include <savefile.h>
#include <DateConverter.h>

#define TehranUTC 12600

/* Macro for changing output-pin in future */
#define OUTPUT_PIN    	LED_BUILTIN
#define OUTPUT_STATE_1  (!ScheduleArray[0].state)
#define OUTPUT_STATE_2	(!LEDstate)

Schedule_t ScheduleArray[50] = {};
uint8_t ScheduleCount = 0;

uint8_t LEDstate = 1;

Ticker timer1;

PersianDate pd;




void handle_Root()
{
	server.send(200, "text/html", "<h1>Connected</h1>");
}

void handle_GetStatus()
{
	JsonDocument doc;
	doc["SSID"] = WiFi.softAPSSID();
	doc["IP"] = WiFi.softAPIP().toString();
	doc["LED State"] = !(LEDstate) ? "high" : "low";

	String str;
	serializeJson(doc, str);
	server.send(200, "application/json", str);
}

void timer1_callback()
{
	digitalWrite(OUTPUT_PIN, OUTPUT_STATE_2);
}

void handle_SetTimer()
{
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
	{
		server.send(400, "text/html", "<h1>invalid json</h1>");
		return;
	}

	if(doc["seconds"].is<float_t>())
	{
		float seconds = doc["seconds"].as<float_t>();
		if(doc["state"].is<uint8_t>())
		{
			LEDstate = doc["state"].as<uint8_t>();
			timer1.once(seconds, timer1_callback);
			server.send(200, "text/html", "<h1>changing in " + String(seconds) + " seconds</h1>");
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid state</h1>");
		}
	}
	else
	{
		server.send(400, "text/html", "<h1>invalid number of seconds</h1>");
	}
}

void handle_SetSysTimestamp()
{
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
	{
		server.send(400, "text/html", "<h1>invalid json</h1>");
		return;
	}
	else
	{

		if(doc["timestamp"].is<time_t>())
		{
			systemtimestamp = doc["timestamp"].as<time_t>();
			SetupSysTimestampIncrement();
			//RTC_SetTimestamp(systemtimestamp);
			server.send(200, "text/html", "<h1>timestamp set</h1>");
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid timestamp</h1>");
		}
	}
}

void handle_GetDate()
{
	UpdateSysDate_systemTS();
	PersianDate pd;
	pd.setGregorianDate(systemDate.year, systemDate.month, systemDate.day);
	pd.convertGregorianToPersian();

	JsonDocument doc;
	doc["Timestamp"] = systemtimestamp;

	doc["miladi"]["year"] = systemDate.year;
	doc["miladi"]["month"] = systemDate.month;
	doc["miladi"]["day"] = systemDate.day;
	doc["miladi"]["hour"] = systemDate.hour;
	doc["miladi"]["minute"] = systemDate.minute;
	doc["miladi"]["second"] = systemDate.second;
	doc["miladi"]["date"] = pd.getGregorianDateString();

	struct tm miladi{};
	time_t stamp = time(nullptr);
	localtime_r(&stamp, &miladi);
	Date_t shamsi1 = gregorianToPersian_ChatGPT(miladi);
	Date_t shamsi2 = gregorianToPersian_Claude(miladi);
	doc["shamsi"]["date1"] = String(shamsi1.year) + "/" + String(shamsi1.month) + "/" + String(shamsi1.day);
	doc["shamsi"]["date2"] = String(shamsi2.year) + "/" + String(shamsi2.month) + "/" + String(shamsi2.day);
	doc["shamsi"]["date3"] = pd.getFullPersianDateString();
	doc["shamsi"]["hour"] = systemDate.hour;
	doc["shamsi"]["minute"] = systemDate.minute;
	doc["shamsi"]["second"] = systemDate.second;

	String str;
	serializeJson(doc, str);
	server.send(200, "application/json", str);
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

bool RemoveSchedule(uint16_t id)
{
	for(uint8_t i = 0; i < ScheduleCount; i++)
	{
		if(ScheduleArray[i].id == id)
		{
			ScheduleArray[i] = Schedule_t{};
			sortSchedules();
			ScheduleCount--;
			saveSchedulesFile();
			return true;
		}
	}

	return false;
}

void handle_RemoveSchedule()
{
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
	{
		server.send(400, "text/html", "<h1>invalid json</h1>");
		return;
	}
	else
	{
		if(doc["id"].is<uint16_t>())
		{
			if(RemoveSchedule(doc["id"]))
			{
				server.send(200, "text/html", "<h1>schedule removed</h1>");
			}
			else
			{
				server.send(400, "text/html", "<h1>schedule not found</h1>");
			}
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid id</h1>");
		}
	}
}

bool AddSchedule(time_t timestamp, uint8_t state, Repeat_t interval, uint16_t id)
{
	if(ScheduleCount < 50)
	{
		ScheduleArray[ScheduleCount].ScheduleTimeStamp = timestamp;

		if(state == 0 || state == 1)
		{
			ScheduleArray[ScheduleCount].state = state;
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid state</h1>");
			return false;
		}
		
		if(once <= interval && interval <= monthly)
		{
			ScheduleArray[ScheduleCount].interval = interval;
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid interval</h1>");
			return false;
		}
		
		for(uint8_t i = 0; i < ScheduleCount; i++)
		{
			if(ScheduleArray[i].id == id)
			{
				server.send(400, "text/html", "<h1>id already exists</h1>");
				return false;
			}
			else
			{
				ScheduleArray[ScheduleCount].id = id;
			}
		}

		ScheduleArray[ScheduleCount].flag = true;
		ScheduleCount++;
		sortSchedules();
		saveSchedulesFile();
		return true;
	}
	else
	{
		server.send(400, "text/html", "<h1>no space</h1>");
		return false;
	}
}

void handle_SetSchedule()
{
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
		{
			server.send(400, "text/html", "<h1>invalid json</h1>");
			return;
		}
	else
	{
		if(doc["scheduletimestamp"].is<time_t>() && 
		   doc["state"].is<uint8_t>() && 
		   doc["interval"].is<uint8_t>() && 
		   doc["id"].is<uint16_t>()) 
		{
			if(AddSchedule(doc["scheduletimestamp"], doc["state"], static_cast<Repeat_t>(doc["interval"]), doc["id"]))
			{
				server.send(200, "text/html", "<h1>schedule added</h1>");
			}
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid keys</h1>");
		}
	}
}

uint32_t handleMonthlySchedules(Schedule_t& Schedule)
{
	struct tm miladi{};
	time_t timestamp = Schedule.ScheduleTimeStamp;
	localtime_r(&timestamp, &miladi);
	Date_t shamsi = gregorianToPersian_Claude(miladi);

	if(shamsi.day <= 29)
	{
		if(shamsi.month <= 6)
		{
			return (31 * 86400);
		}
		else if(shamsi.month <= 11)
		{
			return (30 * 86400);
		}
		else if(shamsi.month == 12)
		{
			if(isPersianLeapYear_Claude(shamsi.year))
			{
				return (30 * 86400);
			}
			else
			{
				return (29 * 86400);
			}
		}
	}

	if(shamsi.day == 30)
	{
		if(shamsi.month == 11 && !isPersianLeapYear_Claude(shamsi.year))
		{
			return ((30 + 29) * 86400);
		}
		else
		{
			return (30 * 86400);
		}
	}

	if(shamsi.day == 31)
	{
		if(1 <= shamsi.month && shamsi.month <= 5)
		{
			return (31 * 86400);
		}
		else if(shamsi.month == 6)
		{
			return ((31 + (5 * 30) + (isPersianLeapYear_Claude(shamsi.year) ? 30 : 29)) * 86400);
		}
	}

	//for warning purposes, should never reach here
	return 0;
}

void ProcessSchedules()
{
	if(ScheduleCount == 0) return;
	if(ScheduleArray[0].flag == false) return;
	if(ScheduleArray[0].ScheduleTimeStamp > systemtimestamp) return;

	time_t delay = systemtimestamp - ScheduleArray[0].ScheduleTimeStamp;

	if(delay > 60)
	{
		switch(ScheduleArray[0].interval)
		{
			case once:
				ScheduleArray[0] = Schedule_t{};
				ScheduleCount--;
				break;
			case daily:
				do{
					ScheduleArray[0].ScheduleTimeStamp += 86400;
				} while(ScheduleArray[0].ScheduleTimeStamp < systemtimestamp);
				break;
			case weekly:
				do{
					ScheduleArray[0].ScheduleTimeStamp += (7 * 86400);
				} while(ScheduleArray[0].ScheduleTimeStamp < systemtimestamp);
				break;
			case monthly:
				do{
					ScheduleArray[0].ScheduleTimeStamp += handleMonthlySchedules(ScheduleArray[0]);
				} while(ScheduleArray[0].ScheduleTimeStamp < systemtimestamp);
				break;
		}

		sortSchedules();
		saveSchedulesFile();
		return;
	}

	digitalWrite(OUTPUT_PIN, OUTPUT_STATE_1);

	switch(ScheduleArray[0].interval)
	{
		case once:
			ScheduleArray[0] = Schedule_t{};
			ScheduleCount--;
			break;
		case daily:
			ScheduleArray[0].ScheduleTimeStamp += 86400;
			break;
		case weekly:
			ScheduleArray[0].ScheduleTimeStamp += (7 * 86400);
			break;
		case monthly:
			ScheduleArray[0].ScheduleTimeStamp += handleMonthlySchedules(ScheduleArray[0]);
			break;
	}

	sortSchedules();
	saveSchedulesFile();
}

void handle_GetSchedules()
{
    JsonDocument doc;

	doc["SystemTimestamp"] = systemtimestamp;
	doc["ScheduleCount"] = ScheduleCount;

    JsonArray array = doc["schedules"].to<JsonArray>();

    for (uint8_t i = 0; i < ScheduleCount; i++) {

        JsonObject obj = array.add<JsonObject>();

		obj["id"] = ScheduleArray[i].id;
        obj["scheduletimestamp"] = ScheduleArray[i].ScheduleTimeStamp;
		obj["state"] = ScheduleArray[i].state;
        obj["interval"] = ScheduleArray[i].interval;
        obj["flag"] = ScheduleArray[i].flag;
    }

    String str;
    serializeJson(doc, str);

    server.send(200, "application/json", str);
}

void handle_ResetSchedules()
{
	for(uint8_t i = 0; i < 50; i++)
	{
		ScheduleArray[i] = Schedule_t{};
	}

	ScheduleCount = 0;
	server.send(200, "text/html", "<h1>schedules reset</h1>");
	saveSchedulesFile();
}

void handle_GetSavedFile()
{
	    if (!LittleFS.exists("/schedules.json")) {
        server.send(400, "text/html", "<h1>File not found</h1>");
        return;
    }

    File file = LittleFS.open("/schedules.json", "r");

    if (!file) {
        server.send(400, "text/html", "<h1>Failed to open file</h1>");
        return;
    }

	server.streamFile(file, "application/json");

	file.close();
}
