#include <log.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <RTC.h>


void Log(const String& message)
{
    File file = LittleFS.open(LOG_FILE, "a");

    if(!file) return;

    file.print("[");
    file.print(String(systemtimestamp));
    file.print(" | ");
    file.print(String(systemDate.year) + "/" + String(systemDate.month) + "/" + String(systemDate.day));
    file.print(" | ");
    file.print(String(systemDate.hour) + ":" + String(systemDate.minute) + ":" + String(systemDate.second));
    file.print("] ");
    file.println(message);

    file.flush();
    file.close();
}