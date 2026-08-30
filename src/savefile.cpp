#include <savefile.h>

#include <ArduinoJson.h>
#include <api_handlers.h>
#include <RTC.h>
#include <scheduler.h>

#define SCHEDULE_FILE       "/schedules.json"
#define TMP_SCHEDULE_FILE   "/schedules.tmp"

#define MAX_SCHEDULES 50


void mountFile()
{
    if(!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed");
    }
    else
    {
        Serial.println("LittleFS mounted");

        if(loadSchedulesFile())
        {
            Serial.println("Schedules loaded successfully");
        }
        else
        {
            Serial.println("Failed to load schedules");
        }
    }
}


static bool replaceFile(const char* temporaryFile, const char* destinationFile)
{
    if (LittleFS.exists(destinationFile) &&
        !LittleFS.remove(destinationFile)) {
        Serial.printf("Failed to remove %s\n", destinationFile);
        return false;
    }

    if (!LittleFS.rename(temporaryFile, destinationFile)) {
        Serial.printf("Failed to rename %s to %s\n",
                      temporaryFile,
                      destinationFile);

        LittleFS.remove(temporaryFile);
        return false;
    }

    return true;
}


bool loadSchedulesFile()
{
    ScheduleCount = 0;

    if (!LittleFS.exists(SCHEDULE_FILE)) {
        Serial.println("Schedule file does not exist");
        return false;
    }

    File file = LittleFS.open(SCHEDULE_FILE, "r");

    if (!file) {
        Serial.println("Failed to open schedule file");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse schedule file: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray schedules = doc["schedules"].as<JsonArray>();

    if (schedules.isNull()) {
        Serial.println("Schedule array is missing");
        return false;
    }

    for (JsonObject schedule : schedules) {
        if (ScheduleCount >= MAX_SCHEDULES) {
            Serial.println("Schedule limit reached");
            break;
        }

        ScheduleArray[ScheduleCount].id =
            schedule["id"] | 0;

        ScheduleArray[ScheduleCount].ScheduleTimeStamp =
            schedule["scheduletimestamp"].as<time_t>();
            //schedule["scheduletimestamp"] | time_t(0);

        ScheduleArray[ScheduleCount].state =
            schedule["state"] | 0;

        ScheduleArray[ScheduleCount].interval =
            static_cast<Repeat_t>(schedule["interval"] | 0);

        ScheduleArray[ScheduleCount].flag =
            schedule["flag"] | false;

        ScheduleCount++;
    }

    Serial.print("Loaded schedules: ");
    Serial.println(ScheduleCount);

    return true;
}


bool saveSchedulesFile()
{
    File file = LittleFS.open(TMP_SCHEDULE_FILE, "w");

    if (!file) {
        Serial.println("Failed to open temporary schedule file");
        return false;
    }

    JsonDocument doc;
    JsonArray schedules = doc["schedules"].to<JsonArray>();

    for (uint8_t i = 0; i < ScheduleCount && i < MAX_SCHEDULES; i++) {
        JsonObject schedule = schedules.add<JsonObject>();

        schedule["id"] = ScheduleArray[i].id;
        schedule["scheduletimestamp"] = ScheduleArray[i].ScheduleTimeStamp;
        schedule["state"] = ScheduleArray[i].state;
        schedule["interval"] = ScheduleArray[i].interval;
        schedule["flag"] = ScheduleArray[i].flag;
    }

    const size_t written = serializeJson(doc, file);
    file.close();

    if (written == 0) {
        Serial.println("Failed to write schedule file");
        LittleFS.remove(TMP_SCHEDULE_FILE);
        return false;
    }

    if (!replaceFile(TMP_SCHEDULE_FILE, SCHEDULE_FILE)) {
        return false;
    }

    Serial.println("Schedules saved");
    return true;
}


bool removeFile(String path)
{
    if (!LittleFS.exists(path)) {
        return true;
    }

    if (!LittleFS.remove(path)) {
        Serial.print("Failed to remove file: ");
        Serial.println(path);
        return false;
    }

    Serial.print("File removed: ");
    Serial.println(path);
    return true;
}