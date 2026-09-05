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
#include <statusLED.h>


uint8_t RelayState;
Ticker timer1_task;
Ticker timer2_wifi;

void handle_Root()
{
	server.send(200, "text/html", "<h1>Connected</h1>");
}

void handle_GetStatus()
{
	JsonDocument doc;
	doc["SSID"] = WiFi.softAPSSID();
	doc["IP"] = WiFi.softAPIP().toString();
	switch (WiFi.getMode())
	{
		case WIFI_OFF:
			doc["Mode"] = "OFF";
			break;
		case WIFI_STA:
			doc["Mode"] = "STA";
			break;
		case WIFI_AP:
			doc["Mode"] = "AP";
			break;
		case WIFI_AP_STA:
			doc["Mode"] = "AP+STA";
			break;
	}
	
	doc["Relay State"] = (digitalRead(RELAY_PIN)) ? "high" : "low";

	String str;
	serializeJson(doc, str);
	server.send(200, "application/json", str);
}

void timer1_callback()
{
	digitalWrite(RELAY_PIN, RelayState);
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
			RelayState = doc["state"].as<uint8_t>();
			timer1_task.once(doc["seconds"].as<float_t>(), timer1_callback);
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
			UpdateSysDate_systemTS();

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
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, server.arg("plain"));

	if(error)
	{
		server.send(400, "text/html", "<h1>invalid json</h1>");
		return;
	}
	else
	{
		if(doc["id"].is<uint8_t>())
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
		   doc["id"].is<uint8_t>()) 
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

void handle_wifisetting()
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
		if(doc["esp_ssid"].is<String>()   && doc["esp_passwd"].is<String>()
		&& doc["modem_ssid"].is<String>() && doc["modem_passwd"].is<String>())
		{
			if(((doc["esp_ssid"].as<String>().length() > 32 || (doc["esp_passwd"].as<String>().length() > 64 
													   		||  doc["esp_passwd"].as<String>().length() < 8)))
														
			|| (doc["modem_ssid"].as<String>().length() > 32 || (doc["modem_passwd"].as<String>().length() > 64 
													   		 ||  doc["modem_passwd"].as<String>().length() < 8)))
			{
				
				server.send(400, "text/html", "<h1>invalid ssid or password length</h1>");
				return;
			}
			else
			{
				AP_SSID = doc["esp_ssid"].as<String>();
				AP_PASSWD = doc["esp_passwd"].as<String>();
				STA_SSID = doc["modem_ssid"].as<String>();
				STA_PASSWD = doc["modem_passwd"].as<String>();
			}
		}
		else
		{
			server.send(400, "text/html", "<h1>invalid key values</h1>");
			return;
		}

		if(saveWifiConfig())
		{
			server.send(200, "text/html", "<h1>changes takes effect after reset</h1>");
		}
		else
		{
			server.send(400, "text/html", "<h1>error changing ssid or password</h1>");
		}
	}
}

void timer2_callback()
{
	wifi_toggle_mode();
	setLEDInterval(50);
	resetLEDInterval(2.f);
}

void handle_wifitoggle()
{
	timer2_wifi.once(5.f, timer2_callback);
	server.send(200, "text/html", "<h1>wifi changing in 5 seconds</h1>");
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
	}
	else
	{
		server.send(400, "text/html", "<h1>error removing log file</h1>");
	}
}

void handle_ChangeLang()
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
		if(doc["language"].is<String>())
		{
			if(doc["language"] == "persian")
			{
				LogLang = persian;
				server.send(200, "text/html", "<h1>زبان سیستم به فارسی تغییر کرد</h1>");
				return;
			}

			if (doc["language"] == "english")
			{
				LogLang = english;
				server.send(200, "text/html", "<h1>language changed to English</h1>");
				return;
			}
			
			server.send(400, "text/html", "<h1>invalid language</h1>");
		}
	}
}