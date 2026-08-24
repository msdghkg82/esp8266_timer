#include <Arduino.h>

#include <Webserver.h>
#include <RTC.h>
#include <savefile.h>

// put function declarations here:


void setup() {
	Serial.begin(115200);
	Serial.println();
  Serial.println("test");
  // put your setup code here, to run once:
  if(!LittleFS.begin())
  {
    Serial.println("LittleFS mount failed");
  }
  else
  {
    Serial.println("LittleFS mounted");
    loadSchedulesFile();
  }

  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
	RTC_setTimestamp(systemtimestamp);
	WebserverSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  RTC_cc();
	WebserverHandleClients();
}

// put function definitions here:
