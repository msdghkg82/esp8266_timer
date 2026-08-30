#include <api_handlers.h>

/* Built-in Libraries */
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <time.h>
#include <algorithm>
/**********************/

#include <RTC.h>
#include <Webserver.h>
#include <savefile.h>
#include <DateConverter.h>
#include <scheduler.h>


#define TehranUTC 12600


uint8_t LEDstate = 1;
Ticker timer1;


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
	digitalWrite(LED_BUILTIN, !LEDstate);
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

	if(doc["seconds"].is<float_t>() && doc["state"].is<uint8_t>())
	{
		if(doc["state"] == 0 || doc["state"] == 1)
		{
			LEDstate = doc["state"].as<uint8_t>();
			timer1.once(doc["seconds"].as<float_t>(), timer1_callback);
			server.send(200, "text/html", "<h1>changing in " + String(doc["seconds"].as<float_t>()) + " seconds</h1>");
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid state</h1>");
		}
	}
	else
	{
		server.send(400, "text/html", "<h1>invalid keys</h1>");
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
			RTC_SetTimestamp(doc["timestamp"].as<time_t>());
			SysTS_RTC();
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

	JsonDocument doc;
	doc["Timestamp"] = systemtimestamp;

	doc["miladi"]["year"] = systemDate.year;
	doc["miladi"]["month"] = systemDate.month;
	doc["miladi"]["day"] = systemDate.day;
	doc["miladi"]["hour"] = systemDate.hour;
	doc["miladi"]["minute"] = systemDate.minute;
	doc["miladi"]["second"] = systemDate.second;
	doc["miladi"]["date"] = String(systemDate.year) + "/" + String(systemDate.month) + "/" + String(systemDate.day);

	Date_t shamsi1 = gregorianToPersian_ChatGPT(systemDate.year, systemDate.month, systemDate.day);
	Date_t shamsi2 = gregorianToPersian_Claude(systemDate.year, systemDate.month, systemDate.day);
	doc["shamsi"]["date1"] = String(shamsi1.year) + "/" + String(shamsi1.month) + "/" + String(shamsi1.day);
	doc["shamsi"]["date2"] = String(shamsi2.year) + "/" + String(shamsi2.month) + "/" + String(shamsi2.day);
	doc["shamsi"]["hour"] = systemDate.hour;
	doc["shamsi"]["minute"] = systemDate.minute;
	doc["shamsi"]["second"] = systemDate.second;

	String str;
	serializeJson(doc, str);
	server.send(200, "application/json", str);
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
			else
			{
				server.send(200, "text/html", "<h1>invalid key values</h1>");
			}
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid keys</h1>");
		}
	}
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
	if(ResetSchedules())
	{
		server.send(200, "text/html", "<h1>schedules reset</h1>");
	}
	else
	{
		server.send(200, "text/html", "<h1>schedules reset but didnt save</h1>");
	}
	
}

void handle_GetSavedFile()
{
	if (!LittleFS.exists(SCHEDULE_FILE)) 
	{
        server.send(400, "text/html", "<h1>File not found</h1>");
        return;
    }

    File file = LittleFS.open(SCHEDULE_FILE, "r");

    if (!file) {
        server.send(400, "text/html", "<h1>Failed to open file</h1>");
        return;
    }

	server.streamFile(file, "application/json");

	file.close();
}
