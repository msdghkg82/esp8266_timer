#include <api_handlers.h>

/* Built-in Libraries */
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <time.h>
/**********************/

#include <RTC.h>
#include <Webserver.h>
#include <savefile.h>
#include <DateConverter.h>
#include <scheduler.h>
#include <wifi.h>
#include <log.h>


#define TehranUTC 12600

uint8_t RelayState;
Ticker timer1_task;
Ticker timer2_wifi;

void handle_Root()
{
	server.send(200, "text/html", "<h1>Connected</h1>");
}

void handle_GetStatus()
{
	Log("===Get Status===");
	JsonDocument doc;
	doc["SSID"] = WiFi.softAPSSID();
	doc["IP"] = WiFi.softAPIP().toString();
	if(WiFi.getMode() == WIFI_OFF)
	{
		doc["wifi"] = "WiFi off";
	}
	else if (WiFi.getMode() == WIFI_AP)
	{
		doc["wifi"] = "WiFi on";
	}
	
	doc["Relay State"] = (digitalRead(RELAY_PIN)) ? "high" : "low";

	String str;
	serializeJson(doc, str);
	server.send(200, "application/json", str);
}

void timer1_callback()
{
	digitalWrite(RELAY_PIN, RelayState);
	Log("Relay state changed to " + String(RelayState));
}

void handle_SetTimer()
{
	Log("===Set Timer===");
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
	{
		server.send(400, "text/html", "<h1>invalid json</h1>");
		Log("invalid json for set timer");
		return;
	}

	if(doc["seconds"].is<float_t>() && doc["state"].is<uint8_t>())
	{
		if(doc["state"] == 0 || doc["state"] == 1)
		{
			RelayState = doc["state"].as<uint8_t>();
			timer1_task.once(doc["seconds"].as<float_t>(), timer1_callback);
			server.send(200, "text/html", "<h1>changing in " + String(doc["seconds"].as<float_t>()) + " seconds</h1>");
			Log("Relay state will change to " + String(RelayState) + " in " + String(doc["seconds"].as<float_t>()) + " seconds");
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid state</h1>");
			Log("invalid state for set timer");
		}
	}
	else
	{
		server.send(400, "text/html", "<h1>invalid keys</h1>");
		Log("invalid keys for set timer");
	}
}

void handle_SetSysTimestamp()
{
	Log("===Set System Timestamp===");
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
	{
		server.send(400, "text/html", "<h1>invalid json</h1>");
		Log("invalid json for set system timestamp");
		return;
	}
	else
	{
		if(doc["timestamp"].is<time_t>())
		{
			RTC_SetTimestamp(doc["timestamp"].as<time_t>());
			SysTS_RTC();
			server.send(200, "text/html", "<h1>timestamp set</h1>");
			Log("timestamp set to: " + String(doc["timestamp"].as<time_t>()));
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid timestamp</h1>");
			Log("invalid timestamp for set system timestamp");
		}
	}
}

void handle_GetDate()
{
	Log("===Get Date===");
	UpdateSysDate_systemTS();
	Date_t shamsi = gregorianToPersian_Claude(systemDate.year, systemDate.month, systemDate.day);

	JsonDocument doc;
	doc["Timestamp"] = systemtimestamp;

	doc["miladi"]["year"] = systemDate.year;
	doc["miladi"]["month"] = systemDate.month;
	doc["miladi"]["day"] = systemDate.day;
	doc["miladi"]["hour"] = systemDate.hour;
	doc["miladi"]["minute"] = systemDate.minute;
	doc["miladi"]["second"] = systemDate.second;
	doc["miladi"]["date"] = String(systemDate.year) + "/" + String(systemDate.month) + "/" + String(systemDate.day);

	doc["shamsi"]["year"] = shamsi.year;
	doc["shamsi"]["month"] = shamsi.month;
	doc["shamsi"]["day"] = shamsi.day;
	doc["shamsi"]["hour"] = systemDate.hour;
	doc["shamsi"]["minute"] = systemDate.minute;
	doc["shamsi"]["second"] = systemDate.second;
	doc["shamsi"]["date"] = String(shamsi.year) + "/" + String(shamsi.month) + "/" + String(shamsi.day);

	String str;
	serializeJson(doc, str);
	server.send(200, "application/json", str);
}

void handle_RemoveSchedule()
{
	Log("===Remove Schedule===");
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
	{
		server.send(400, "text/html", "<h1>invalid json</h1>");
		Log("invalid json for remove schedule");
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
				Log("schedule not found");
			}
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid id</h1>");
			Log("invalid id for remove schedule");
		}
	}
}

void handle_SetSchedule()
{
	Log("===Set Schedule===");
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));
	if(error)
		{
			server.send(400, "text/html", "<h1>invalid json</h1>");
			Log("invalid json for set schedule");
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
				Log("invalid key values for schedule");
			}
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid keys</h1>");
			Log("invalid keys for schedule");
		}
	}
}

void handle_GetSchedules()
{
	Log("===Get Schedules===");
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
	Log("Schedules Sent");
}

void handle_ResetSchedules()
{
	Log("===Reset Schedules===");
	if(ResetSchedules())
	{
		server.send(200, "text/html", "<h1>schedules reset</h1>");
	}
	else
	{
		server.send(200, "text/html", "<h1>schedules reset but didnt save</h1>");
		Log("schedules reset but didnt save");
	}
	
}

void handle_wifisetting()
{
	Log("===WiFi Setting===");
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));
	if(error)
	{
		server.send(400, "text/html", "<h1>invalid json</h1>");
		Log("invalid json for wifi setting");
		return;
	}
	else
	{
		if(doc["ssid"].is<String>() && doc["passwd"].is<String>())
		{
			if(doc["ssid"].as<String>().length() > 32 || (doc["passwd"].as<String>().length() > 64 
													   && doc["passwd"].as<String>().length() < 8))
			{
				server.send(400, "text/html", "<h1>invalid ssid or password length</h1>");
				Log("invalid ssid or password length");
				return;
			}

			SSID = doc["ssid"].as<String>();
			PASSWD = doc["passwd"].as<String>();
			
			if(saveWifiConfig())
			{
				server.send(200, "text/html", "<h1>changes takes effect after reset</h1>");
				Log("wifi settings changed, ssid: " + SSID + ", password: " + PASSWD);
			}
			else
			{
				server.send(400, "text/html", "<h1>error changing ssid or password</h1>");
				Log("error changing ssid or password");
			}
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid keys</h1>");
			Log("invalid keys for wifi setting");
		}
	}
}

void timer2_callback()
{
	wifi_off();
	Log("wifi turned off");
}

void handle_wifioff()
{
	Log("===WiFi Off===");
	timer2_wifi.attach(5.f, timer2_callback);
	server.send(200, "text/html", "<h1>wifi turning off in 5 seconds</h1>");
	Log("wifi turning off in 5 seconds");
}

void handle_GetLog()
{
	if(!LittleFS.exists(LOG_FILE))
    {
        server.send(404,"text/html", "<h1>Log file not found</h1>");
        return;
    }

    File file = LittleFS.open(LOG_FILE, "r");

	server.sendHeader("Content-Disposition", "attachment; filename=log.txt");
    server.streamFile(file, "application/octet-stream");

    file.close();
}

void handle_RemoveLog()
{
	if(removeFile(LOG_FILE))
	{
		server.send(200, "text/html", "<h1>log file removed</h1>");
		Log("log file removed");
	}
	else
	{
		server.send(400, "text/html", "<h1>error removing log file</h1>");
		Log("error removing log file");
	}
}