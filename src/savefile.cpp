#include <savefile.h>

#include <ArduinoJson.h>
#include <api_handlers.h>

#define SCHEDULE_FILE "/schedules.json"

bool loadSchedulesFile()
{
    if (!LittleFS.exists(SCHEDULE_FILE)) {
        Serial.println("Schedule file does not exist");

        ScheduleCount = 0;
        return true;
    }

    File file = LittleFS.open(SCHEDULE_FILE, "r");

    if (!file) {
        Serial.println("Failed to open schedule file");
        return false;
    }

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, file);

    file.close();

    if (error) 
    {
        Serial.print("Failed to parse schedule file: ");
        Serial.println(error.c_str());

        ScheduleCount = 0;
        return false;
    }

    JsonArray schedules = doc["schedules"].as<JsonArray>();

    ScheduleCount = 0;

    for (JsonObject schedule : schedules) {

        if (ScheduleCount >= 50)
            break;

        ScheduleArray[ScheduleCount].id = schedule["id"] | 0;
        ScheduleArray[ScheduleCount].ScheduleTimeStamp = schedule["timestamp"] | 0;
        ScheduleArray[ScheduleCount].state = schedule["state"] | 0;
        ScheduleArray[ScheduleCount].interval = (Repeat_t)(schedule["interval"] | 0);
        if(ScheduleArray[ScheduleCount].interval == monthly)
        {
            ScheduleArray[ScheduleCount].dayOfMonth = schedule["dayOfMonth"] | 0;
        }
        ScheduleArray[ScheduleCount].flag = schedule["flag"] | 0;

        ScheduleCount++;
    }

    Serial.print("Loaded schedules: ");
    Serial.println(ScheduleCount);

    return true;
}

bool saveSchedulesFile()
{
    File file = LittleFS.open(SCHEDULE_FILE, "w");

    if(!file)
    {
        Serial.println("failed to open file to write");
        return false;
    }

    JsonDocument doc;
    JsonArray schedules = doc["schedules"].to<JsonArray>();

    for(uint8_t i = 0; i < ScheduleCount; i++)
    {
        JsonObject schedule = schedules.add<JsonObject>();

        schedule["id"] = ScheduleArray[i].id;
        schedule["timestamp"] = ScheduleArray[i].ScheduleTimeStamp;
        schedule["state"] = ScheduleArray[i].state;
        schedule["interval"] = ScheduleArray[i].interval;
        if(ScheduleArray[i].interval == monthly)
        {
            schedule["dayOfMonth"] = ScheduleArray[i].dayOfMonth;
        }
        schedule["flag"] = ScheduleArray[i].flag;
    }

    if(serializeJson(doc, file) == 0)
    {
        Serial.println("failed to write file");
        file.close();
        return false;
    }

    file.close();

    Serial.println("Schedules saved");

    return true;
}
