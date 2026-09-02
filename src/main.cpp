#include <Arduino.h>

#include <Webserver.h>
#include <RTC.h>
#include <savefile.h>
#include <scheduler.h>
#include <button.h>
#include <statusLED.h>

// put function declarations here:


void setup() {
	Serial.begin(115200);
	Serial.println();
  Serial.println("Turned On");

  // put your setup code here, to run once:

  mountLittleFS();
  LoadFiles();
  RTC_Begin();
  SysTS_RTC();
  UpdateSysDate_systemTS();
	WebserverSetup();
  Scheduler_Init();
  ButtonInit();
  statusLED_Init();
}

void loop() {
  // put your main code here, to run repeatedly:
  incrementSysTimestamp();
  //UpdateSysDate_systemTS();
	WebserverHandleClients();
  ProcessSchedules();
  ButtonTask();
}

// put function definitions here:
